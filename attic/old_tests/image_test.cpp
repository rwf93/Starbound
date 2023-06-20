#include "StarImage.hpp"
#include "StarImageProcessing.hpp"
#include "StarFormat.hpp"
#include "StarAssets.hpp"

using namespace Star;

int main(int argc, char** argv) {
  if (argc != 4) {
    coutf("usage <image> <operationlist> <output>\n");
    return 1;
  }

  try {
    ImagePtr image = Image::loadPng(argv[1]);
    ImageProcessor processor = ImageProcessor(argv[2]);

    for (auto ref : processor.imageReferences())
      processor.setImageReference(ref, Image::loadPng(ref.utf8Ptr()));

    coutf("processing parameters: %s\n", processor.printOperations());
    auto result = processor.process(image);
    coutf("done\n");

    result->writePng(argv[3]);

    return 0;
  } catch (std::exception const& e) {
    coutf("Exception caught: %s\n", e.what());
    return 1;
  }
}
