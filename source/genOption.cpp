#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0]
         << " prefix level[=1] csize[=2] threshold[=0.7] wsize[=7]"
         << " minImageNum[=3] CPU[=8]" << std::endl
         << std::endl
         << "Parameters are for PMVS2 option files." << std::endl
         << "Please refer to the following URL for the definitions:" << std::endl
         << "http://grail.cs.washington.edu/software/pmvs/documentation.html" << std::endl;
    exit (1);
  }

  int level = 1;
  if (3 <= argc)
    level = atoi(argv[2]);

  int csize = 2;
  if (4 <= argc)
    csize = atoi(argv[3]);

  float threshold = 0.7;
  if (5 <= argc)
    threshold = atof(argv[4]);

  int wsize = 7;
  if (6 <= argc)
    wsize = atoi(argv[5]);

  int minImageNum = 3;
  if (7 <= argc)
    minImageNum = atoi(argv[6]);

  int CPU = 8;
  if (8 <= argc)
    CPU = atoi(argv[7]);

  const int setEdge = 0;
  const int useBound = 0;
  const int useVisData = 1;
  const int sequence = -1;

  std::ifstream ifstr;
  char ske[1024];
  sprintf(ske, "%sske.dat", argv[1]);
  ifstr.open(ske);

  std::string header;
  int inum, cnum;
  ifstr >> header >> inum >> cnum;

  std::ofstream script;
  char pmvsfile[1024];
#ifdef WIN32 // PM in order to get launchable script on windows
  sprintf(pmvsfile, "%spmvs.bat", argv[1]);
#else
  sprintf(pmvsfile, "%spmvs.sh", argv[1]);
#endif
  script.open(pmvsfile);

  for (int c = 0; c < cnum; ++c) {
    std::ofstream ofstr;
    char buffer[1024];
    sprintf(buffer, "%soption-%04d", argv[1], c);
    ofstr.open(buffer);

    sprintf(buffer, "pmvs2 pmvs/ option-%04d", c);
    script << buffer << std::endl;

    ofstr << "# generated by genOption. mode 1. cluster: " << c << std::endl
          << "level " << level << std::endl
          << "csize " << csize << std::endl
          << "threshold " << threshold << std::endl
          << "wsize " << wsize << std::endl
          << "minImageNum " << minImageNum << std::endl
          << "CPU " << CPU << std::endl
          << "setEdge " << setEdge << std::endl
          << "useBound " << useBound << std::endl
          << "useVisData " << useVisData << std::endl
          << "sequence " << sequence << std::endl
          << "maxAngle 10" << std::endl
          << "quad 2.0" << std::endl;

    int timagenum, oimagenum;
    ifstr >> timagenum >> oimagenum;

    std::vector<int> timages, oimages;
    timages.resize(timagenum);
    oimages.resize(oimagenum);
    for (int i = 0; i < timagenum; ++i)
      ifstr >> timages[i];
    for (int i = 0; i < oimagenum; ++i)
      ifstr >> oimages[i];

    ofstr << "timages " << timagenum << ' ';
    for (int i = 0; i < timagenum; ++i)
      ofstr << timages[i] << ' ';
    ofstr << std::endl;
    ofstr << "oimages " << oimagenum << ' ';
    for (int i = 0; i < oimagenum; ++i)
      ofstr << oimages[i] << ' ';
    ofstr << std::endl;
    ofstr.close();
  }
  ifstr.close();
  script << std::endl;
  script.close();
}
