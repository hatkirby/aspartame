#include "vendor/csv.h"
#include "identifier.h"
#include "histogram.h"
#include <rawr.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <string>
#include <iostream>
#include <sstream>



using speakerstore = identifier<std::string>;
using speaker_id = speakerstore::key_type;


struct speaker_data {

  std::string name;
  histogram<speaker_id> nextSpeaker;
  rawr chain;

};




int main(int, char**)
{
  srand(time(NULL));
  rand(); rand(); rand(); rand();

  speakerstore speakers;
  std::map<speaker_id, speaker_data> speakerData;
  histogram<speaker_id> allSpeakers;



  io::CSVReader<2,io::trim_chars<' ', '\t'>,io::double_quote_escape<',', '"'>> in("../dialogue.csv");
  std::string speaker;
  std::string line;

  bool hadPrev = false;
  speaker_id prevSpeaker;

  while (in.read_row(speaker, line))
  {
    speaker_id spId = speakers.add(speaker);
    speaker_data& myData = speakerData[spId];
    myData.name = speaker;

    allSpeakers.add(spId);

    if (hadPrev && prevSpeaker != spId)
    {
      speaker_data& psd = speakerData[prevSpeaker];
      psd.nextSpeaker.add(spId);
    }

    myData.chain.addCorpus(line);

    hadPrev = true;
    prevSpeaker = spId;
  }

  for (auto& sp : speakerData)
  {
    sp.second.chain.compile(4);
    sp.second.nextSpeaker.compile();
  }

  std::cout << "Speakers:" << std::endl;
  for (auto& sp : speakerData)
  {
    std::cout << "  " << sp.second.name << std::endl;
  }
  std::cout << std::endl;

  allSpeakers.compile();

  for (;;)
  {
    speaker_id curSpeaker = allSpeakers.next();

    std::ostringstream theEnd;

    for (int i = 0; i < 5; i++)
    {
      speaker_data& curSd = speakerData.at(curSpeaker);

      //std::ostringstream thisLine;

      if (curSd.name != "")
      {
        theEnd << curSd.name << ": ";
      }

      theEnd << curSd.chain.randomSentence(1);

      /*if (i > 0 && theEnd.str().length() + thisLine.str().length() > 280)
      {
        break;
      }*/

      theEnd << std::endl;
      //theEnd << thisLine.str();

      curSpeaker = curSd.nextSpeaker.next();
    }

    std::string output = theEnd.str();
    output.resize(280);
    output = output.substr(0, output.find_last_of('\n'));
    std::cout << output;

    std::cout << std::endl;
    std::cout << std::endl;

    getc(stdin);
  }
}
