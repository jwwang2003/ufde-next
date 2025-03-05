#ifndef PLCCARRYINFER_H
#define PLCCARRYINFER_H

#include "plc_utils.h"

#include <map>

namespace COS {
class TDesign;
}

namespace FDU {
namespace Place {

using namespace COS;

class PLCInstance;
class PLCModule;
struct ChainNode;
struct ChainEdge;

/************************************************************************/
/* carry chain�еĽڵ�֮��ıߣ����Կ���ȥ��                            */
/************************************************************************/
struct ChainEdge {
  ChainNode *_from_node;
  ChainNode *_to_node;

  ChainEdge(ChainNode *f = nullptr, ChainNode *t = nullptr)
      : _from_node(f), _to_node(t) {}
};

/************************************************************************/
/* carry chain�еĽڵ㶨��                                              */
/************************************************************************/
struct ChainNode {
  PLCInstance *_owner;   // ָ���slice��ָ��
  ChainEdge *_from_edge; // ��slice��from��
  ChainEdge *_to_edge;   // ��slice��to��

  explicit ChainNode(PLCInstance *owner)
      : _owner(owner), _from_edge(nullptr), _to_edge(nullptr) {}
  ~ChainNode() { delete _to_edge; }
  // �����ýڵ��to edge�����������ڵ����Ӻ�
  void create_to_edge(ChainNode *t) {
    t->_from_edge = _to_edge = new ChainEdge(this, t);
  }
};

/************************************************************************/
/* ��������carry chain                                                  */
/************************************************************************/
class CarryChainInference {
public:
  ~CarryChainInference() {
    for (ChainNodes::value_type &node : _nodes)
      delete node.second;
  }
  // map<name, insts>
  typedef std::map<std::string, std::vector<PLCInstance *>> CarryChains;
  // �ҵ�design�����еĸ���carry chain,���Ҵ洢��chains
  void inference(TDesign *design, CarryChains &chains);

protected:
  // ����carry chain�е�һ���ڵ�
  ChainNode *create_node(PLCInstance *owner);
  // �ҵ�design�����е�carry chain�ĸ����ڵ㣬����û�����ó�carry chain
  void find_chains(PLCModule *top_cell);
  // �������ڵ���֯�ɸ���carry chainȻ��洢�������ú�����
  void store_chains(CarryChains &chains);

private:
  typedef std::map<PLCInstance *, ChainNode *> ChainNodes;
  ChainNodes _nodes;
};

} // namespace Place
} // namespace FDU

#endif