#include <iostream>
#include "WebRemoveDuplicates.h"

using namespace std;

int main() {
    WebRemoveDuplicates deduper;

    deduper.readInfoFromFile("../data/newripepage.dat", "../data/newoffset.dat");

    deduper.cutRedundantPages();

    deduper.writeBackToFile("../data/deduplicatedWebData.dat", "../data/deduplicatedOffsetData.dat");
    
    cout << "Deduplication and writing back completed.\n";

    return 0;
}