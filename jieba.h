#ifndef CJIEBA_JIEBA_H
#define CJIEBA_JIEBA_H

#include <stdlib.h>
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* Jieba;

typedef struct {
  size_t offset;
  size_t len;
} Word;

typedef enum {
  DefaultMode = 0,
  SearchMode = 1,
} TokenizeMode;

Jieba NewJieba(const char* dict_content,
      const char* hmm_content, 
      const char* user_dict_content,
      const char* idf_content,
      const char* stop_words_content);
void FreeJieba(Jieba);

char** Cut(Jieba handle, const char* sentence, int is_hmm_used);
char** CutAll(Jieba handle, const char* sentence);
char** CutForSearch(Jieba handle, const char* sentence, int is_hmm_used);
char** Tag(Jieba handle, const char* sentence);
void AddWord(Jieba handle, const char* word);
void AddWordEx(Jieba handle, const char* word, int freq, const char* tag);
void RemoveWord(Jieba handle, const char* word);

Word* Tokenize(Jieba x, const char* sentence, TokenizeMode mode, int is_hmm_used);

struct CWordWeight {
  char* word;
  double weight;
};

char** Extract(Jieba handle, const char* sentence, int top_k);
struct CWordWeight* ExtractWithWeight(Jieba handle, const char* sentence, int top_k);
void FreeWordWeights(struct CWordWeight* wws);

#ifdef __cplusplus
}
#endif

#endif // CJIEBA_JIEBA_H
