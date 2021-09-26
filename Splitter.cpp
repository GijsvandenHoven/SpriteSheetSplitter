#include <vector>
#include <cassert>
#include "Splitter.h"

void Splitter::work(std::vector <SplitterOpts> &jobs) {
    std::vector<unsigned char> image;
    image.reserve(16*16*16*32); // reserves enough space for a 16x16 spritesheet with 32 rows. (256x512)
    int completedCount = 0;
    for (auto& job : jobs) {

        bool inPathOK = ssio.setInPath(job.inDirectory, job.isPNGInDirectory, job.recursive);
        bool outPathOK = ssio.setOutPath(job.outDirectory);
        // only work the job if the input is OK. The only input error that can happen from now on is a png decode error.
        if (! (inPathOK && outPathOK)) continue;

        if (job.isPNGInDirectory) {
            completedCount += splitNext(image);
        } else {
            int work = job.workAmount;
            while (work-- > 0 && ssio.hasNextPNG()) {
                completedCount += splitNext(image);
            }
        }
    }
}

/**
 *
 * @param imgBuffer A buffer to hold the image. This is first cleared when the function is called.
 *                  It is not created within the function to prevent re-allocating memory lots of times.
 * @return whether the image was successfully split
 */
bool Splitter::splitNext(std::vector<unsigned char>& imgBuffer) {
    imgBuffer.clear();
    if (! ssio.findNextPNG()) return false; // no more PNGs to work on in this path.

    unsigned int error = ssio.load(imgBuffer);
    if (error) {
        std::cout << "[ERROR] LodePNG decode error: " << error << ". (Most likely a corrupt png)\n"; // if it's an incorrect path at this point then that is a bug!
    }



    imgBuffer.clear();
    return true;
}