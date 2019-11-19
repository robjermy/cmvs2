#include <iostream>
#include <fstream>
#include "../image/image.hpp"
#include "detectFeatures.hpp"
#include "harris.hpp"
#include "dog.hpp"
#include "point.hpp"

using namespace PMVS3;
using namespace std;
using namespace Image;

CDetectFeatures::CDetectFeatures(void) {
  mtx_init(&_rwlock, mtx_plain | mtx_recursive);
}

CDetectFeatures::~CDetectFeatures() {
  mtx_destroy(&_rwlock);
}

void CDetectFeatures::run(const CPhotoSetS& pss, const int num,
                          const int csize, const int level,
                          const int CPU) {
  _ppss = &pss;
  _csize = csize;
  _level = level;
  _CPU = CPU;

  _points.clear();
  _points.resize(num);
  
  //----------------------------------------------------------------------
  for (int index = 0; index < num; ++index)
    _jobs.push_back(index);
  
  vector<thrd_t> threads(_CPU);
  for (int i = 0; i < _CPU; ++i)
    thrd_create(&threads[i], &runThreadTmp, (void*)this);
  for (int i = 0; i < _CPU; ++i)
    thrd_join(threads[i], NULL);
  //----------------------------------------------------------------------
  cerr << "done" << endl;
}

int CDetectFeatures::runThreadTmp(void* arg) {
  CDetectFeatures* detectFeatures = (CDetectFeatures*)arg;  
  detectFeatures->runThread();
  return 0;
}

void CDetectFeatures::runThread(void) {
  while (1) {
    int index = -1;
    mtx_lock(&_rwlock);
    if (!_jobs.empty()) {
      index = _jobs.front();
      _jobs.pop_front();
    }
    mtx_unlock(&_rwlock);
    if (index == -1)
      break;
    
    const int image = _ppss->_images[index];
    cerr << image << ' ' << flush;

    //?????????????  May need file lock, because targetting images
    //should not overlap among multiple processors.    
    char buffer[1024];
    sprintf(buffer, "%smodels/%08d.affin%d", _ppss->_prefix.c_str(), image, _level);
    ifstream ifstr;
    ifstr.open(buffer);
    if (ifstr.is_open()) {
      ifstr.close();
      continue;
    }
    ifstr.close();
    
    //----------------------------------------------------------------------
    // parameters
    // for harris
    const float sigma = 4.0f;
    // for DoG
    const float firstScale = 1.0f;    const float lastScale = 3.0f;

    //----------------------------------------------------------------------
    // Harris
    {
      CHarris harris;
      multiset<CPoint> result;
      harris.run(_ppss->_photos[index].getImage(_level),
                 _ppss->_photos[index].CImage::getMask(_level),
                 _ppss->_photos[index].CImage::getEdge(_level),
                 _ppss->_photos[index].getWidth(_level),
                 _ppss->_photos[index].getHeight(_level), _csize, sigma, result);
      
      multiset<CPoint>::reverse_iterator rbegin = result.rbegin();
      while (rbegin != result.rend()) {
        _points[index].push_back(*rbegin);
        rbegin++;
      }
    }

    //----------------------------------------------------------------------
    // DoG
    {
      CDifferenceOfGaussians dog;
      multiset<CPoint> result;
      dog.run(_ppss->_photos[index].getImage(_level),
              _ppss->_photos[index].CImage::getMask(_level),
              _ppss->_photos[index].CImage::getEdge(_level),
              _ppss->_photos[index].getWidth(_level),
              _ppss->_photos[index].getHeight(_level),
              _csize, firstScale, lastScale, result);
      
      multiset<CPoint>::reverse_iterator rbegin = result.rbegin();      
      while (rbegin != result.rend()) {
        _points[index].push_back(*rbegin);
        rbegin++;
      }
    }
  }
}
