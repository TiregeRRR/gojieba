#ifndef CPPJIEBA_DICT_TRIE_HPP
#define CPPJIEBA_DICT_TRIE_HPP

#include "Trie.hpp"
#include "Unicode.hpp"
#include "limonp/Logging.hpp"
#include "limonp/StringUtil.hpp"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdint.h>
#include <string>

namespace cppjieba {

using namespace limonp;

const double MIN_DOUBLE = -3.14e+100;
const double MAX_DOUBLE = 3.14e+100;
const size_t DICT_COLUMN_NUM = 3;
const char *const UNKNOWN_TAG = "";

class DictTrie {
public:
  enum UserWordWeightOption {
    WordWeightMin,
    WordWeightMedian,
    WordWeightMax,
  }; // enum UserWordWeightOption

  DictTrie(const string &dict_string = "", const string &user_dict_strings = "",
           UserWordWeightOption user_word_weight_opt = WordWeightMedian) {
    Init(dict_string, user_dict_strings, user_word_weight_opt);
  }

  ~DictTrie() { delete trie_; }

  bool InsertUserWord(const string &word, const string &tag = UNKNOWN_TAG) {
    DictUnit node_info;
    if (!MakeNodeInfo(node_info, word, user_word_default_weight_, tag)) {
      return false;
    }
    active_node_infos_.push_back(node_info);
    trie_->InsertNode(node_info.word, &active_node_infos_.back());
    return true;
  }

  bool InsertUserWord(const string &word, int freq,
                      const string &tag = UNKNOWN_TAG) {
    DictUnit node_info;
    double weight =
        freq ? log(1.0 * freq / freq_sum_) : user_word_default_weight_;
    if (!MakeNodeInfo(node_info, word, weight, tag)) {
      return false;
    }
    active_node_infos_.push_back(node_info);
    trie_->InsertNode(node_info.word, &active_node_infos_.back());
    return true;
  }

  bool DeleteUserWord(const string &word, const string &tag = UNKNOWN_TAG) {
    DictUnit node_info;
    if (!MakeNodeInfo(node_info, word, user_word_default_weight_, tag)) {
      return false;
    }
    trie_->DeleteNode(node_info.word, &node_info);
    return true;
  }

  const DictUnit *Find(RuneStrArray::const_iterator begin,
                       RuneStrArray::const_iterator end) const {
    return trie_->Find(begin, end);
  }

  void Find(RuneStrArray::const_iterator begin,
            RuneStrArray::const_iterator end, vector<struct Dag> &res,
            size_t max_word_len = MAX_WORD_LENGTH) const {
    trie_->Find(begin, end, res, max_word_len);
  }

  bool Find(const string &word) {
    const DictUnit *tmp = NULL;
    RuneStrArray runes;
    if (!DecodeRunesInString(word, runes)) {
      XLOG(ERROR) << "Decode failed.";
    }
    tmp = Find(runes.begin(), runes.end());
    if (tmp == NULL) {
      return false;
    } else {
      return true;
    }
  }

  bool IsUserDictSingleChineseWord(const Rune &word) const {
    return IsIn(user_dict_single_chinese_word_, word);
  }

  double GetMinWeight() const { return min_weight_; }

  void InserUserDictNode(const string &line) {
    vector<string> buf;
    DictUnit node_info;
    Split(line, buf, " ");
    if (buf.size() == 1) {
      MakeNodeInfo(node_info, buf[0], user_word_default_weight_, UNKNOWN_TAG);
    } else if (buf.size() == 2) {
      MakeNodeInfo(node_info, buf[0], user_word_default_weight_, buf[1]);
    } else if (buf.size() == 3) {
      int freq = atoi(buf[1].c_str());
      assert(freq_sum_ > 0.0);
      double weight = log(1.0 * freq / freq_sum_);
      MakeNodeInfo(node_info, buf[0], weight, buf[2]);
    }
    static_node_infos_.push_back(node_info);
    if (node_info.word.size() == 1) {
      user_dict_single_chinese_word_.insert(node_info.word[0]);
    }
  }

  void LoadUserDict(const vector<string> &buf) {
    for (size_t i = 0; i < buf.size(); i++) {
      InserUserDictNode(buf[i]);
    }
  }

  void LoadUserDict(const set<string> &buf) {
    std::set<string>::const_iterator iter;
    for (iter = buf.begin(); iter != buf.end(); iter++) {
      InserUserDictNode(*iter);
    }
  }

  void LoadUserDict(const string &dictStrings) {
    vector<string> dicts = limonp::Split(dictStrings, "|;");
    size_t lineno = 0;
    for (size_t i = 0; i < dicts.size(); i++) {
      istringstream f(dicts[i]);
      string line;

      for (; getline(f, line); lineno++) {
        if (line.size() == 0) {
          continue;
        }
        InserUserDictNode(line);
      }
    }
  }

private:
  void Init(const string &dict_string, const string &user_dict_string,
            UserWordWeightOption user_word_weight_opt) {
    LoadDict(dict_string);
    freq_sum_ = CalcFreqSum(static_node_infos_);
    CalculateWeight(static_node_infos_, freq_sum_);
    SetStaticWordWeights(user_word_weight_opt);

    if (user_dict_string.size()) {
      LoadUserDict(user_dict_string);
    }
    Shrink(static_node_infos_);
    CreateTrie(static_node_infos_);
  }

  void CreateTrie(const vector<DictUnit> &dictUnits) {
    assert(dictUnits.size());
    vector<Unicode> words;
    vector<const DictUnit *> valuePointers;
    for (size_t i = 0; i < dictUnits.size(); i++) {
      words.push_back(dictUnits[i].word);
      valuePointers.push_back(&dictUnits[i]);
    }

    trie_ = new Trie(words, valuePointers);
  }

  bool MakeNodeInfo(DictUnit &node_info, const string &word, double weight,
                    const string &tag) {
    if (!DecodeRunesInString(word, node_info.word)) {
      XLOG(ERROR) << "Decode " << word << " failed.";
      return false;
    }
    node_info.weight = weight;
    node_info.tag = tag;
    return true;
  }

  void LoadDict(const string &dict) {
    istringstream ifs(dict);
    string line;
    vector<string> buf;

    DictUnit node_info;
    for (size_t lineno = 0; getline(ifs, line); lineno++) {
      Split(line, buf, " ");
      XCHECK(buf.size() == DICT_COLUMN_NUM)
          << "split result illegal, line:" << line;
      MakeNodeInfo(node_info, buf[0], atof(buf[1].c_str()), buf[2]);
      static_node_infos_.push_back(node_info);
    }
  }

  static bool WeightCompare(const DictUnit &lhs, const DictUnit &rhs) {
    return lhs.weight < rhs.weight;
  }

  void SetStaticWordWeights(UserWordWeightOption option) {
    XCHECK(!static_node_infos_.empty());
    vector<DictUnit> x = static_node_infos_;
    sort(x.begin(), x.end(), WeightCompare);
    min_weight_ = x[0].weight;
    max_weight_ = x[x.size() - 1].weight;
    median_weight_ = x[x.size() / 2].weight;
    switch (option) {
    case WordWeightMin:
      user_word_default_weight_ = min_weight_;
      break;
    case WordWeightMedian:
      user_word_default_weight_ = median_weight_;
      break;
    default:
      user_word_default_weight_ = max_weight_;
      break;
    }
  }

  double CalcFreqSum(const vector<DictUnit> &node_infos) const {
    double sum = 0.0;
    for (size_t i = 0; i < node_infos.size(); i++) {
      sum += node_infos[i].weight;
    }
    return sum;
  }

  void CalculateWeight(vector<DictUnit> &node_infos, double sum) const {
    assert(sum > 0.0);
    for (size_t i = 0; i < node_infos.size(); i++) {
      DictUnit &node_info = node_infos[i];
      assert(node_info.weight > 0.0);
      node_info.weight = log(double(node_info.weight) / sum);
    }
  }

  void Shrink(vector<DictUnit> &units) const {
    vector<DictUnit>(units.begin(), units.end()).swap(units);
  }

  vector<DictUnit> static_node_infos_;
  deque<DictUnit> active_node_infos_; // must not be vector
  Trie *trie_;

  double freq_sum_;
  double min_weight_;
  double max_weight_;
  double median_weight_;
  double user_word_default_weight_;
  unordered_set<Rune> user_dict_single_chinese_word_;
};
} // namespace cppjieba

#endif
