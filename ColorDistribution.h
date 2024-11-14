#include <opencv2/highgui.hpp>
#include <vector>

using namespace cv;

struct ColorDistribution {
  float data[ 8 ][ 8 ][ 8 ]; // l'histogramme
  int nb;                     // le nombre d'échantillons

  ColorDistribution() { reset(); }
  ColorDistribution& operator=( const ColorDistribution& other ) = default;
  // Met à zéro l'histogramme
  void reset();
  // Ajoute l'échantillon color à l'histogramme:
  // met +1 dans la bonne case de l'histogramme et augmente le nb d'échantillons
  void add( cv::Vec3b color );
  // Indique qu'on a fini de mettre les échantillons:
  // divise chaque valeur du tableau par le nombre d'échantillons
  // pour que case représente la proportion des picxls qui ont cette couleur.
  void finished();
  // Retourne la distance entre cet histogramme et l'histogramme other
  float distance( const ColorDistribution& other ) const;

  float min_distance(const std::vector<ColorDistribution>& hists);

  // Récupère la couleur qui apprait le plus souvent.
  Vec3b most_used_color();

  private:
    const int DATA_SIZE = 8;
};
