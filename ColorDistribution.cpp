#include "ColorDistribution.h"
#include <cmath>

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
  int r = color[2] / 32; // Récupère l'indice de la case en réduisant les
                         // couleurs à 8 niveaux (256/32 = 8)
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
        total += pow(normalize(i,j,k) - other.normalize(i,j,k), 2) / normalize(i,j,k) + other.normalize(i,j,k);
      }
    }
  }

  return total;
}

float ColorDistribution::normalize(int i, int j, int k) const {
  {
    return data[i][j][k] / nb;
  }
}
