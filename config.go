package gojieba

import (
	"path"
	"runtime"

	_ "embed"
)

var (
	DICT_DIR        string
	DICT_PATH       string
	HMM_PATH        string
	USER_DICT_PATH  string
	IDF_PATH        string
	STOP_WORDS_PATH string
)

var (
	//go:embed dict/jieba.dict.utf8
	DICT_DATA string
	//go:embed dict/hmm_model.utf8
	HMM_DATA string
	//go:embed dict/user.dict.utf8
	USER_DICT_DATA string
	//go:embed dict/idf.utf8
	IDF_DATA string
	//go:embed dict/stop_words.utf8
	STOP_WORDS_DATA string
)

func init() {
	DICT_DIR = path.Join(path.Dir(getCurrentFilePath()), "dict")
	DICT_PATH = path.Join(DICT_DIR, "jieba.dict.utf8")
	HMM_PATH = path.Join(DICT_DIR, "hmm_model.utf8")
	USER_DICT_PATH = path.Join(DICT_DIR, "user.dict.utf8")
	IDF_PATH = path.Join(DICT_DIR, "idf.utf8")
	STOP_WORDS_PATH = path.Join(DICT_DIR, "stop_words.utf8")
}

const TOTAL_DICT_PATH_NUMBER = 5

func getDictString() [TOTAL_DICT_PATH_NUMBER]string {
	dicts := [TOTAL_DICT_PATH_NUMBER]string{
		DICT_DATA,
		HMM_DATA,
		USER_DICT_DATA,
		IDF_DATA,
		STOP_WORDS_DATA,
	}

	return dicts
}

func getCurrentFilePath() string {
	_, filePath, _, _ := runtime.Caller(1)
	return filePath
}
