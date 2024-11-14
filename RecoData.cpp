#include "RecoData.h"
#include "opencv2/core/matx.hpp"
#include <iostream>

using namespace std;

void RecoData::clear() {
  color_distributions.clear();
  color = cv::Vec3b(0.,0.,0.);
}

void RecoData::add_cd(ColorDistribution color_distribution) {
  color_distributions.push_back(color_distribution);
  set_color();
}

bool RecoData::is_empty() {
  return color_distributions.empty();
}

int RecoData::size_cbs() {
  return color_distributions.size();
}

void RecoData::set_color()
{
  int r = 0;
  int g = 0;
  int b = 0;

  int total = color_distributions.size();

  for (int i = 0; i < color_distributions.size(); ++i) {
    Vec3b most_used_color = color_distributions[i].most_used_color();

    r += most_used_color[2];
    g += most_used_color[1];
    b += most_used_color[0];
  }

  color = Vec3b( b / total, g / total, r / total );
}
