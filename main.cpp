#include "ColorDistribution.h"
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
    const std::vector<ColorDistribution>
        &col_hists, /*< les distributions de couleurs du fond */
    const std::vector<ColorDistribution>
        &col_hists_object, /*< les distributions de couleurs de l'objet */
    const std::vector<Vec3b> &colors /*< les couleurs pour fond/objet */
) {
  Mat output;
  input.copyTo(output);

  for (int y = 0; y <= HEIGHT - RECO_BLOC; y += RECO_BLOC) {
    for (int x = 0; x <= WIDTH - RECO_BLOC; x += RECO_BLOC) {
      Point pt_start(x, y);
      Point pt_end(x + RECO_BLOC, y + RECO_BLOC);
      ColorDistribution h = getColorDistribution(input, pt_start, pt_end);

      float min_distance_bg = h.minDistance(col_hists);
      float min_distance_obj = h.minDistance(col_hists_object);

      Vec3b color_bloc;
      if (min_distance_obj < min_distance_bg) {
        color_bloc = colors[1];
      } else {
        color_bloc = colors[0];
      }

      for (int i = x; i < x + RECO_BLOC; i++) {
        for (int j = y; j < y + RECO_BLOC; j++) {
          // cout << "Will use color: " << color_bloc << " at index (" << i << ", " << j << ")" << endl;
          output.at<Vec3b>(j, i) = color_bloc;
        }
      }
    }
  }

  return output;
}

int main(int argc, char **argv) {
  Mat img_input, img_seg, img_d_bgr, img_d_hsv, img_d_lab;
  std::vector<ColorDistribution> col_hists;        // histogrammes du fond
  std::vector<ColorDistribution> col_hists_object; // histogrammes de l'objet

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

  vector<Vec3b> colors = {Vec3b(0., 0., 0.), Vec3b(0., 0., 255.)};

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
      col_hists.clear();

      for (int y = 0; y <= HEIGHT - BB_BLOC; y += BB_BLOC) {
        for (int x = 0; x <= WIDTH - BB_BLOC; x += BB_BLOC) {
          Point pt_start(x, y);
          Point pt_end(x + BB_BLOC, y + BB_BLOC);
          ColorDistribution background =
              getColorDistribution(img_input, pt_start, pt_end);
          col_hists.push_back(background);
        }
      }
      int nb_hists_background = col_hists.size();
      cout << "Size of background: " << nb_hists_background << endl;
    }
    if (c == 'a') {
      // Histogramme de couleur de la partie contenu dans le rectangle blanc :
      ColorDistribution white_rectangle_cd =
          getColorDistribution(img_input, pt1, pt2);
      col_hists_object.push_back(white_rectangle_cd);

      cout << "Got histogram of white rectangle. Size of list: " << col_hists_object.size() << endl;
    }
    if (c == 'f') // permet de geler l'image
      freeze = !freeze;

    if (c == 'r') {
      if (col_hists.empty() || col_hists_object.empty()) {
        continue;
      }

      if (reco) {
        // On reset toutes nos informations si on est déjà en mode reco.
        reco = false;
        col_hists.clear();
        col_hists_object.clear();
        cout << "Reco mode deactivated" << endl;
      } else {
        reco = true;
        cout << "Reco mode activated" << endl;
      }
    }

    if (reco) {
      // Mode reconnaissance
      Mat gray;
      cvtColor(img_input, gray, COLOR_BGR2GRAY);
      Mat reco_output =
          recoObject(img_input, col_hists, col_hists_object, colors);

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
