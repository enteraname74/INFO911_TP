#include "ColorDistribution.h"

struct RecoData {
  std::vector<ColorDistribution> color_distributions;
  cv::Vec3b color;

  RecoData(){ clear(); }
  // Initialise un RecoObject.
  void clear();

  // Ajoute un histogramme à l'objet.
  void add_cd(ColorDistribution color_distribution);

  // Recupère le total de color_distributions.
  int size_cbs();

  bool is_empty();

  private:
    // Définie la couleur à utiliser pour afficher l'objet.
    void set_color();
};
