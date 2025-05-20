package gojieba

/*
#cgo CXXFLAGS: -I./deps -DLOGGING_LEVEL=LL_WARNING -O3 -Wall
#include <stdlib.h>
#include "jieba.h"
*/
import "C"

import (
	"runtime"
	"unsafe"
)

type TokenizeMode int

const (
	DefaultMode TokenizeMode = iota
	SearchMode
)

type Word struct {
	Str   string
	Start int
	End   int
}

type Jieba struct {
	jieba C.Jieba
}

func NewJieba() *Jieba {
	dictStrings := getDictString()

	dictContent := dictStrings[0]
	hmmContent := dictStrings[1]
	userDictContent := dictStrings[2]
	idfContent := dictStrings[3]
	stopWordsContent := dictStrings[4]

	cDictContent := C.CString(dictContent)
	cHmmContent := C.CString(hmmContent)
	cUserDictContent := C.CString(userDictContent)
	cIdfContent := C.CString(idfContent)
	cStopWordsContent := C.CString(stopWordsContent)

	defer C.free(unsafe.Pointer(cDictContent))
	defer C.free(unsafe.Pointer(cHmmContent))
	defer C.free(unsafe.Pointer(cUserDictContent))
	defer C.free(unsafe.Pointer(cIdfContent))
	defer C.free(unsafe.Pointer(cStopWordsContent))

	jieba := &Jieba{
		C.NewJieba(
			cDictContent,
			cHmmContent,
			cUserDictContent,
			cIdfContent,
			cStopWordsContent,
		),
	}
	runtime.SetFinalizer(jieba, (*Jieba).Free)
	return jieba
}

func (x *Jieba) Free() {
	C.FreeJieba(x.jieba)
}

func (x *Jieba) Cut(s string, hmm bool) []string {
	c_int_hmm := 0
	if hmm {
		c_int_hmm = 1
	}
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	var words **C.char = C.Cut(x.jieba, cstr, C.int(c_int_hmm))
	defer C.FreeWords(words)
	res := cstrings(words)
	return res
}

func (x *Jieba) CutAll(s string) []string {
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	var words **C.char = C.CutAll(x.jieba, cstr)
	defer C.FreeWords(words)
	res := cstrings(words)
	return res
}

func (x *Jieba) CutForSearch(s string, hmm bool) []string {
	c_int_hmm := 0
	if hmm {
		c_int_hmm = 1
	}
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	var words **C.char = C.CutForSearch(x.jieba, cstr, C.int(c_int_hmm))
	defer C.FreeWords(words)
	res := cstrings(words)
	return res
}

func (x *Jieba) Tag(s string) []string {
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	var words **C.char = C.Tag(x.jieba, cstr)
	defer C.FreeWords(words)
	res := cstrings(words)
	return res
}

func (x *Jieba) AddWord(s string) {
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	C.AddWord(x.jieba, cstr)
}

func (x *Jieba) AddWordEx(s string, freq int, tag string) {
	cstr := C.CString(s)
	ctag := C.CString(tag)
	defer C.free(unsafe.Pointer(ctag))
	defer C.free(unsafe.Pointer(cstr))
	C.AddWordEx(x.jieba, cstr, C.int(freq), ctag)
}

func (x *Jieba) RemoveWord(s string) {
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	C.RemoveWord(x.jieba, cstr)
}

func (x *Jieba) Tokenize(s string, mode TokenizeMode, hmm bool) []Word {
	c_int_hmm := 0
	if hmm {
		c_int_hmm = 1
	}
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	var words *C.Word = C.Tokenize(x.jieba, cstr, C.TokenizeMode(mode), C.int(c_int_hmm))
	defer C.free(unsafe.Pointer(words))
	return convertWords(s, words)
}

type WordWeight struct {
	Word   string
	Weight float64
}

func (x *Jieba) Extract(s string, topk int) []string {
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	var words **C.char = C.Extract(x.jieba, cstr, C.int(topk))
	res := cstrings(words)
	defer C.FreeWords(words)
	return res
}

func (x *Jieba) ExtractWithWeight(s string, topk int) []WordWeight {
	cstr := C.CString(s)
	defer C.free(unsafe.Pointer(cstr))
	words := C.ExtractWithWeight(x.jieba, cstr, C.int(topk))
	p := unsafe.Pointer(words)
	res := cwordweights((*C.struct_CWordWeight)(p))
	defer C.FreeWordWeights(words)
	return res
}

func cwordweights(x *C.struct_CWordWeight) []WordWeight {
	var s []WordWeight
	eltSize := unsafe.Sizeof(*x)
	for (*x).word != nil {
		ww := WordWeight{
			C.GoString(((C.struct_CWordWeight)(*x)).word),
			float64((*x).weight),
		}
		s = append(s, ww)
		x = (*C.struct_CWordWeight)(unsafe.Pointer(uintptr(unsafe.Pointer(x)) + eltSize))
	}
	return s
}
