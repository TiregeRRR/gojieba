#ifndef CPPJIEAB_JIEBA_H
#define CPPJIEAB_JIEBA_H

#include "KeywordExtractor.hpp"
#include "QuerySegment.hpp"

namespace cppjieba {

class Jieba {
public:
  Jieba(string dict_string, string model_string, string user_dict_string,
        string idf_string, string stop_word_string)
      : dict_trie_(dict_string, user_dict_string), model_(model_string),
        mp_seg_(&dict_trie_), hmm_seg_(&model_), mix_seg_(&dict_trie_, &model_),
        full_seg_(&dict_trie_), query_seg_(&dict_trie_, &model_),
        extractor(&dict_trie_, &model_, idf_string, stop_word_string) {}
  ~Jieba() {}

  struct LocWord {
    string word;
    size_t begin;
    size_t end;
  }; // struct LocWord

  void Cut(const string &sentence, vector<string> &words,
           bool hmm = true) const {
    mix_seg_.Cut(sentence, words, hmm);
  }
  void Cut(const string &sentence, vector<Word> &words, bool hmm = true) const {
    mix_seg_.Cut(sentence, words, hmm);
  }
  void CutAll(const string &sentence, vector<string> &words) const {
    full_seg_.Cut(sentence, words);
  }
  void CutAll(const string &sentence, vector<Word> &words) const {
    full_seg_.Cut(sentence, words);
  }
  void CutForSearch(const string &sentence, vector<string> &words,
                    bool hmm = true) const {
    query_seg_.Cut(sentence, words, hmm);
  }
  void CutForSearch(const string &sentence, vector<Word> &words,
                    bool hmm = true) const {
    query_seg_.Cut(sentence, words, hmm);
  }
  void CutHMM(const string &sentence, vector<string> &words) const {
    hmm_seg_.Cut(sentence, words);
  }
  void CutHMM(const string &sentence, vector<Word> &words) const {
    hmm_seg_.Cut(sentence, words);
  }
  void CutSmall(const string &sentence, vector<string> &words,
                size_t max_word_len) const {
    mp_seg_.Cut(sentence, words, max_word_len);
  }
  void CutSmall(const string &sentence, vector<Word> &words,
                size_t max_word_len) const {
    mp_seg_.Cut(sentence, words, max_word_len);
  }

  void Tag(const string &sentence, vector<pair<string, string>> &words) const {
    mix_seg_.Tag(sentence, words);
  }
  string LookupTag(const string &str) const { return mix_seg_.LookupTag(str); }
  bool InsertUserWord(const string &word, const string &tag = UNKNOWN_TAG) {
    return dict_trie_.InsertUserWord(word, tag);
  }

  bool InsertUserWord(const string &word, int freq,
                      const string &tag = UNKNOWN_TAG) {
    return dict_trie_.InsertUserWord(word, freq, tag);
  }

  bool DeleteUserWord(const string &word, const string &tag = UNKNOWN_TAG) {
    return dict_trie_.DeleteUserWord(word, tag);
  }

  bool Find(const string &word) { return dict_trie_.Find(word); }

  void ResetSeparators(const string &s) {
    // TODO
    mp_seg_.ResetSeparators(s);
    hmm_seg_.ResetSeparators(s);
    mix_seg_.ResetSeparators(s);
    full_seg_.ResetSeparators(s);
    query_seg_.ResetSeparators(s);
  }

  const DictTrie *GetDictTrie() const { return &dict_trie_; }

  const HMMModel *GetHMMModel() const { return &model_; }

  void LoadUserDict(const vector<string> &buf) { dict_trie_.LoadUserDict(buf); }

  void LoadUserDict(const set<string> &buf) { dict_trie_.LoadUserDict(buf); }

  void LoadUserDict(const string &path) { dict_trie_.LoadUserDict(path); }

private:
  DictTrie dict_trie_;
  HMMModel model_;

  // They share the same dict trie and model
  MPSegment mp_seg_;
  HMMSegment hmm_seg_;
  MixSegment mix_seg_;
  FullSegment full_seg_;
  QuerySegment query_seg_;

public:
  KeywordExtractor extractor;
}; // class Jieba

} // namespace cppjieba

#endif // CPPJIEAB_JIEBA_H
