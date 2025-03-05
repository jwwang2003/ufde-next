#include "cil/siteLib/site/cfgInSite.h"
#include "log.h"
#include "utils.h"
#include "utils/expCalc.h"

namespace FDU {
namespace cil_lib {
using namespace boost;

void funcInCfgElemSite::getContentsOfFunc(vecBits &bits,
                                          const cfgElem &cfgElement) {
  // 		ASSERTD(
  // 			_quomodo == cfgElement._cfgElemFunc._quomodo,
  // 			CilException("function in cfgElem of site: quomodo not
  // equal ... " + _quomodo + "!=" + cfgElement._cfgElemFunc._quomodo)
  // 		);

  if (_quomodo == "naming") {
    for (sramInCfgElemSite *sram : _srams) {
      bitTile tBit;
      tBit._basicCell = sram->_basicCell;
      tBit._sramName = sram->_name;
      tBit._bitContent = sram->_content;
      bits.push_back(tBit);
    }
  } else if (_quomodo == "equation" || _quomodo == "srambit") {
    int addrSize = 0;
    for (sramInCfgElemSite *sram : _srams) {
      if (sram->_optAddr >= 0) {
        ++addrSize;
      }
    }
    std::vector<int> truthTab =
        Exp2LUT(cfgElement._cfgElemFunc._name, addrSize).toVec();
    for (sramInCfgElemSite *sram : _srams) {
      if (sram->_optAddr >= 0) {
        bitTile tBit;
        tBit._basicCell = sram->_basicCell;
        tBit._sramName = sram->_name;
        int sramAddr = sram->_optAddr;
        ASSERTD(sramAddr >= 0 && sramAddr < addrSize,
                "function in cfgElem of site: invalid sram address");
        tBit._bitContent = truthTab[sramAddr];
        bits.push_back(tBit);
      } else {
        bitTile tBit;
        tBit._basicCell = sram->_basicCell;
        tBit._sramName = sram->_name;
        tBit._bitContent = sram->_content;
        bits.push_back(tBit);
      }
    }
  } else
    throw CilException("quomodo of function in site: invalid quomodo ... " +
                       _quomodo);
}

void cfgElemSite::getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement) {
  funcInCfgElemSite *func = getFunc(cfgElement);
  if (func)
    func->getContentsOfFunc(bits, cfgElement);
  // "else" can be omit when release
  // 		else
  // 			cout << Warning("cil: not exist function named " +
  // cfgElement._cfgElemFunc._name + " for " + cfgElement._cfgElemName) << endl;
}

void cfgElemsSite::getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement) {
  cfgElemSite *elem = cfgElems().find(cfgElement._cfgElemName);
  if (elem)
    elem->getContentsOfFunc(bits, cfgElement);
}

void cfgInfoSite::getContentsOfFunc(vecBits &bits, const cfgElem &cfgElement) {
  _cfgElems.getContentsOfFunc(bits, cfgElement);
}

funcInCfgElemSite *cfgElemSite::getFunc(const cfgElem &cfgElement) {
  string quomodo = cfgElement._cfgElemFunc._quomodo;
  if (quomodo == "equation" || quomodo == "srambit") {
    functionIter it =
        find_if(functions(), [&quomodo](const funcInCfgElemSite *func) {
          return func->getQuomodo() == quomodo;
        });
    ASSERTD(it != _functions.end(),
            "cfgElem: not exist function with quomodo = equation or srambit");
    return *it;
  } else { // _quomodo == "naming"
    return getFunc(cfgElement._cfgElemFunc._name);
  }
}

void cfgElemSite::listDfotCfgs(vecCfgs &cfgs) {
#ifdef _DEBUG
  // 		BOOST_FOREACH(funcInCfgElemSite& func, functions()){
  // 			if (func.isDefault()) ++dfotFuncCount;
  // 		}
  // int dfotFuncCount = count_if(functions(), [](const funcInCfgElemSite* func)
  // { return func->isDefault(); }); ASSERTD(dfotFuncCount == 1,
  // BstrException("cfgElem in site: I have more than 1(or 0) default
  // function"));
#endif
  // funcInCfgElemSite& dfotFunc = *find_if(functions(), [](const
  // funcInCfgElemSite* func) { return func->isDefault(); }); cfgElem cfg;
  // cfg._cfgElemName = _name; dfotFunc.transform(cfg._cfgElemFunc);
  // cfgs.push_back(cfg);
  functionIter it = find_if(functions(), [](const funcInCfgElemSite *func) {
    return func->isDefault();
  });
  if (it != _functions.end()) {
    cfgElem cfg;
    cfg._cfgElemName = _name;
    it->transform(cfg._cfgElemFunc);
    cfgs.push_back(cfg);
  }
}

void cfgElemsSite::listDfotCfgs(vecCfgs &cfgs) {
  for (cfgElemSite *cfg : cfgElems())
    cfg->listDfotCfgs(cfgs);
}

void cfgInfoSite::listDfotCfgs(vecCfgs &cfgs) { _cfgElems.listDfotCfgs(cfgs); }

} // namespace cil_lib
} // namespace FDU