#include "ColorDistribution.h"
#include <cmath>
#include <iterator>

using namespace std;

void ColorDistribution::reset() {
  nb = 0;

  for (int i = 0; i < DATA_SIZE; ++i) {
    for (int j = 0; j < DATA_SIZE; ++j) {
      for (int k = 0; k < DATA_SIZE; ++k) {
        data[i][j][k] = 0.;
      }
    }
  }
}

void ColorDistribution::add(cv::Vec3b color) {
  // Récupère l'indice de la case en réduisant les
  // couleurs à 8 niveaux (256/32 = 8)
  int r = color[2] / 32;
  int g = color[1] / 32;
  int b = color[0] / 32;

  // Incrémente la case correspondante dans l'histogramme et augmente le nombre
  // d'échantillons
  data[r][g][b] += 1.0f;
  nb += 1;
}

void ColorDistribution::finished() {
  for (int i = 0; i < DATA_SIZE; ++i) {
    for (int j = 0; j < DATA_SIZE; ++j) {
      for (int k = 0; k < DATA_SIZE; ++k) {
        data[i][j][k] = data[i][j][k] / nb;
      }
    }
  }
}

float ColorDistribution::distance(const ColorDistribution &other) const {
  float total = 0.;

  for (int i = 0; i < DATA_SIZE; ++i) {
    for (int j = 0; j < DATA_SIZE; ++j) {
      for (int k = 0; k < DATA_SIZE; ++k) {

        float denominator = data[i][j][k] + other.data[i][j][k];

        if (denominator > 0) {
          auto res = pow(data[i][j][k] - other.data[i][j][k], 2) / denominator;
          total += res;
        }
      }
    }
  }

  return total;
}

float ColorDistribution::minDistance(const std::vector<ColorDistribution>& hists) {
  if (hists.empty()) return -1.;

  float current_min = distance(hists[0]);
  for (int i = 1; i < hists.size(); ++i) {
    float current_distance = distance(hists[i]);
    if (current_distance < current_min) {
      current_min = current_distance;
    }
  }

  return current_min;
}
