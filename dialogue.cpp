#include "vendor/csv.h"
#include "identifier.h"
#include "histogram.h"
#include <rawr.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <twitter.h>
#include <yaml-cpp/yaml.h>

using speakerstore = identifier<std::string>;
using speaker_id = speakerstore::key_type;

struct speaker_data {

  std::string name;
  histogram<speaker_id> nextSpeaker;
  rawr chain;
  size_t count = 0;

};

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "usage: garnet [configfile]" << std::endl;
    return -1;
  }

  std::random_device randomDevice;
  std::mt19937 rng(randomDevice());

  std::string configfile(argv[1]);
  YAML::Node config = YAML::LoadFile(configfile);

  twitter::auth auth(
    config["consumer_key"].as<std::string>(),
    config["consumer_secret"].as<std::string>(),
    config["access_key"].as<std::string>(),
    config["access_secret"].as<std::string>());

  twitter::client client(auth);

  speakerstore speakers;
  std::map<speaker_id, speaker_data> speakerData;
  histogram<speaker_id> allSpeakers;

  using csv =
    io::CSVReader<
      2,
      io::trim_chars<' ', '\t'>,
      io::double_quote_escape<',', '"'>>;

  csv in(config["transcript"].as<std::string>());
  std::string speaker;
  std::string line;

  bool hadPrev = false;
  speaker_id prevSpeaker;

  while (in.read_row(speaker, line))
  {
    speaker_id spId = speakers.add(speaker);
    speaker_data& myData = speakerData[spId];
    myData.name = speaker;
    myData.count++;

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
    std::cout << "  " << sp.second.name << ": " << sp.second.count << std::endl;
  }
  std::cout << std::endl;

  allSpeakers.compile();

  for (;;)
  {
    std::set<speaker_id> pastSpeakers;

    speaker_id curSpeaker = allSpeakers.next(rng);

    std::ostringstream theEnd;
    int maxLines = std::uniform_int_distribution<int>(3, 6)(rng);

    for (int i = 0; i < maxLines; i++)
    {
      pastSpeakers.insert(curSpeaker);

      speaker_data& curSd = speakerData.at(curSpeaker);

      if (curSd.name != "")
      {
        theEnd << curSd.name << ": ";
      }

      int maxL = std::uniform_int_distribution<int>(1, 30)(rng);
      std::string curLine = curSd.chain.randomSentence(maxL, rng);

      if (curSd.name == "" &&
          curLine[0] != '[' &&
          curLine[0] != '(' &&
          curLine[0] != '*')
      {
        theEnd << "[" << curLine << "]";
      } else {
        theEnd << curLine;
      }

      theEnd << std::endl;

      int psi =
        std::uniform_int_distribution<int>(0, pastSpeakers.size()-1)(rng);

      speaker_id repeatSpeaker = *std::next(std::begin(pastSpeakers), psi);
      if (repeatSpeaker != curSpeaker &&
          std::bernoulli_distribution(1.0 / 3.0)(rng))
      {
        curSpeaker = repeatSpeaker;
      } else {
        curSpeaker = curSd.nextSpeaker.next(rng);
      }
    }

    std::string output = theEnd.str();
    output.resize(280);
    output = output.substr(0, output.find_last_of('\n'));
    std::cout << output;

    try
    {
      client.updateStatus(output);
    } catch (const twitter::twitter_error& error)
    {
      std::cout << "Twitter error while tweeting: " << error.what()
        << std::endl;
    }

    std::cout << std::endl;
    std::cout << std::endl;

    std::this_thread::sleep_for(std::chrono::hours(4));
  }
}
