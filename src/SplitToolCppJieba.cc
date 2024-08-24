#include "SplitToolCppJieba.h"

const char* const DICT_PATH = "../conf/dict/jieba.dict.utf8";
const char* const HMM_PATH = "../conf/dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../conf/dict/user.dict.utf8";
const char* const IDF_PATH = "../conf/dict/idf.utf8";
const char* const STOP_WORD_PATH = "../conf/dict/stop_words.utf8";

// 这里定义，写在头文件里面，会有重定义的问题，编译不过
cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);  

vector<string> SplitToolCppJieba::cut(const string &input) {
    vector<string> words;
    jieba.Cut(input, words, true);
    return words;
}