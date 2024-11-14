#include "RecoData.h"
#include "opencv2/core/matx.hpp"
#include "opencv2/core/types.hpp"
#include <cstdio>
#include <iostream>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace cv;
using namespace std;

const int BB_BLOC = 128;
const int RECO_BLOC = 8;
const int WIDTH = 640;
const int HEIGHT = 480;
const int SIZE = 50;
const int BACKGROUND_COL = 0;
const int OBJECTS_START_COL = 1;

// Pour basculer d'un objet à l'autre (afin de l'éditer)
const char KEY_UP = 'R';
const char KEY_DOWN = 'T';

Vec3b adaptObjectColorForDisplay(const Vec3b& colorBGR, double saturationScale, double luminosityScale) {
    Mat bgrMat(1, 1, CV_8UC3, Scalar(colorBGR[0], colorBGR[1], colorBGR[2]));

    Mat hsvMat;
    cvtColor(bgrMat, hsvMat, COLOR_BGR2HSV);

    Vec3b hsvColor = hsvMat.at<Vec3b>(0, 0);

    hsvColor[1] = saturate_cast<uchar>(hsvColor[1] * saturationScale);
    hsvColor[2] = saturate_cast<uchar>(hsvColor[2] * luminosityScale);

    hsvMat.at<Vec3b>(0, 0) = hsvColor;

    Mat resultBGR;
    cvtColor(hsvMat, resultBGR, COLOR_HSV2BGR);

    return resultBGR.at<Vec3b>(0, 0);
}

ColorDistribution getColorDistribution(Mat input, Point pt1, Point pt2) {
  ColorDistribution cd;
  for (int y = pt1.y; y < pt2.y; y++)
    for (int x = pt1.x; x < pt2.x; x++) {
      cd.add(input.at<Vec3b>(y, x));
    }
  cd.finished();
  return cd;
}

Mat recoObject(
    Mat input,
    const std::vector< RecoData >& all_col_hists,
    const std::vector<Vec3b> &colors
) {
  Mat output;
  input.copyTo(output);

  for (int y = 0; y <= HEIGHT - RECO_BLOC; y += RECO_BLOC) {
    for (int x = 0; x <= WIDTH - RECO_BLOC; x += RECO_BLOC) {
      Point pt_start(x, y);
      Point pt_end(x + RECO_BLOC, y + RECO_BLOC);
      ColorDistribution h = getColorDistribution(input, pt_start, pt_end);

      float min_distance_bg = h.min_distance(all_col_hists[BACKGROUND_COL].color_distributions);
      float min_distance_obj = h.min_distance(all_col_hists[OBJECTS_START_COL].color_distributions);

      Vec3b color_obj = all_col_hists[OBJECTS_START_COL].color;

      // Recherche de l'objet le plus proche du bloc dans la partie de la liste couvrant tous nos objets identifiés
      for (int i = OBJECTS_START_COL; i < all_col_hists.size(); ++i) {
        RecoData current_reco_data = all_col_hists[i];

        float tmp_distance_obj = h.min_distance(current_reco_data.color_distributions);
        if (tmp_distance_obj < min_distance_obj) {
          min_distance_obj = tmp_distance_obj;
          color_obj = current_reco_data.color;
        }
      }

      Vec3b color_bloc;
      if (min_distance_obj < min_distance_bg) {
        // On adapte la couleur de l'objet pour qu'elle soit toujours bien visible.
        color_bloc = adaptObjectColorForDisplay(color_obj, 10.5, 10.);
      } else {
        color_bloc = colors[0];
      }

      for (int i = x; i < x + RECO_BLOC; i++) {
        for (int j = y; j < y + RECO_BLOC; j++) {
          output.at<Vec3b>(j, i) = color_bloc;
        }
      }
    }
  }

  return output;
}

int main(int argc, char **argv) {
  Mat img_input, img_seg, img_d_bgr, img_d_hsv, img_d_lab;

  std::vector<RecoData> all_col_hists = {RecoData()};
  int current_object_col = OBJECTS_START_COL;

  VideoCapture *pCap = nullptr;

  // Ouvre la camera
  pCap = new VideoCapture(0);
  if (!pCap->isOpened()) {
    cout << "Couldn't open image / camera ";
    return 1;
  }
  // Force une camera 640x480 (pas trop grande).
  pCap->set(CAP_PROP_FRAME_WIDTH, 640);
  pCap->set(CAP_PROP_FRAME_HEIGHT, 480);
  (*pCap) >> img_input;
  if (img_input.empty())
    return 1; // probleme avec la camera

  Point pt1(WIDTH / 2 - SIZE / 2, HEIGHT / 2 - SIZE / 2);
  Point pt2(WIDTH / 2 + SIZE / 2, HEIGHT / 2 + SIZE / 2);

  namedWindow("input", 1);
  imshow("input", img_input);

  bool freeze = false;
  bool reco = false;

  vector<Vec3b> colors = {
    Vec3b(0., 0., 0.),
    Vec3b(0., 0., 255.),
    Vec3b(255., 0., 0.),
  };

  Mat output = img_input;
  while (true) {
    char c = (char)waitKey(50); // attend 50ms -> 20 images/s

    if (pCap != nullptr && !freeze)
      (*pCap) >> img_input;  // récupère l'image de la caméra
    if (c == 27 || c == 'q') // permet de quitter l'application
      break;
    if (c == 'v') {
      Point pt_left_start(0, 0);
      Point pt_left_end(WIDTH / 2, HEIGHT);

      Point pt_right_start(WIDTH / 2, 0);
      Point pt_right_end(WIDTH, HEIGHT);

      ColorDistribution cd_left =
          getColorDistribution(img_input, pt_left_start, pt_left_end);
      ColorDistribution cd_right =
          getColorDistribution(img_input, pt_right_start, pt_right_end);

      float distance = cd_left.distance(cd_right);
      cout << "Distance: " << distance << endl;
    }
    if (c == 'b') {
      // On ne garde pas les précédentes valeurs du tableau :
      if (!all_col_hists[BACKGROUND_COL].is_empty()) {
        all_col_hists[BACKGROUND_COL].clear();
      }

      for (int y = 0; y <= HEIGHT - BB_BLOC; y += BB_BLOC) {
        for (int x = 0; x <= WIDTH - BB_BLOC; x += BB_BLOC) {
          Point pt_start(x, y);
          Point pt_end(x + BB_BLOC, y + BB_BLOC);
          ColorDistribution background =
              getColorDistribution(img_input, pt_start, pt_end);
          all_col_hists[BACKGROUND_COL].add_cd(background);
        }
      }
      int nb_hists_background = all_col_hists[BACKGROUND_COL].size_cbs();
      cout << "Size of background: " << nb_hists_background << endl;
    }
    if (c == 'a') {
      // Histogramme de couleur de la partie contenu dans le rectangle blanc, à l'index choisi par l'utilisateur :
      ColorDistribution white_rectangle_cd =
          getColorDistribution(img_input, pt1, pt2);

      if (current_object_col >= all_col_hists.size()) {
        RecoData object_data = RecoData();
        object_data.add_cd(white_rectangle_cd);
        all_col_hists.push_back(object_data);
      } else {
        all_col_hists[current_object_col].add_cd(white_rectangle_cd);
      }

      cout << "Got histogram of white rectangle. Size of list: " << all_col_hists[current_object_col].size_cbs() << endl;
      cout << "Most used color in white rectangle: " << white_rectangle_cd.most_used_color() << endl;
    }
    if (c == 'f') // permet de geler l'image
      freeze = !freeze;

    if (c == 'r') {
      // On ne rentre pas dans le mode reco si on a pas au moins un fond et un objet.
      if (all_col_hists.size() < 2) {
        continue;
      }

      if (reco) {
        // On reset toutes nos informations si on est déjà en mode reco.
        reco = false;
        all_col_hists.clear();
        cout << "Reco mode deactivated" << endl;
      } else {
        reco = true;
        cout << "Reco mode activated" << endl;
      }
    }

    if (c == KEY_UP) {
      if (current_object_col > OBJECTS_START_COL) {
        current_object_col -= 1;
      }
      cout << "Current object index is: " << current_object_col << endl;
    }
    if (c == KEY_DOWN) {
      current_object_col += 1;
      cout << "Current object index is: " << current_object_col << endl;
    }

    if (reco) {
      // Mode reconnaissance
      Mat gray;
      cvtColor(img_input, gray, COLOR_BGR2GRAY);
      Mat reco_output =
          recoObject(img_input, all_col_hists, colors);

      cvtColor(gray, img_input, COLOR_GRAY2BGR);
      output = 0.5 * reco_output + 0.5 * img_input; // mélange reco + caméra
    } else {
      output = img_input;
      cv::rectangle(img_input, pt1, pt2, Scalar({255.0, 255.0, 255.0}), 1);
    }
    imshow("input", output); // affiche le flux video
  }
  return 0;
}
