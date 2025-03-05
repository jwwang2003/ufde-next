#include "plc_carry_infer.h"
#include "plc_factory.h"
#include "plc_utils.h"

namespace FDU {
namespace Place {

using namespace std;
using namespace boost;

#undef EXPORT_NETLIST

/*	���ܣ��ҵ������е�carry chain,���洢
 *	������design:��������� chains: �ҵ���carry chain
 *����ֵ��void
 *	˵����chainsΪһ��map,��keyΪstring,carry chain�����֣�contentΪvector��Ϊ
 *			���ڲ�instance�ṹ��ָ�뼯��
 */
void CarryChainInference::inference(TDesign *design, CarryChains &chains) {
  find_chains(static_cast<PLCModule *>(design->top_module()));
  store_chains(chains);
#ifdef EXPORT_NETLIST
  design->save(fileio::xml, design->name() + "_carrychain.xml");
#endif
}
/*	���ܣ�����һ��carry chain�еĽڵ㣬���洢��_nodes����
 *	������owner: һ��carry chain �еĽڵ�
 *����ֵ��ChainNode* �������ڵ��ָ��
 *	˵����
 *
 */
ChainNode *CarryChainInference::create_node(PLCInstance *owner) {
  ChainNode *node = nullptr;
  // ���������_nodes���棬��ô���ظýڵ�
  // ��������ڣ�����һ�������뵽_nodes��
  if (_nodes.count(owner))
    node = _nodes[owner];
  else {
    node = new ChainNode(owner);
    _nodes.insert(make_pair(owner, node));
  }
  return node;
}
/*	���ܣ��ҵ������и���carry chain�Ľڵ�
 *	������top_cell: design�Ķ��㵥Ԫ
 *����ֵ��void
 *	˵����������ÿ��net��pin name����Ϊcin,cout��Ϊcarry chain,
 *			�������ֻ���ҵ����о�����Щ���Եĵ㣬������Щ�㼰��ߴ�����
 */
void CarryChainInference::find_chains(PLCModule *top_cell) {
  // �ҵ�������pin��������CIN��COUT������
  PLCInstance *cin_instance;
  PLCInstance *cout_instance;
  for (PLCNet *net : top_cell->nets()) {
    PLCNet::pin_iter cin_pin = find_if(
        net->pins(), [](const Pin *pin) { return pin->name() == DEVICE::CIN; });
    PLCNet::pin_iter cout_pin = find_if(net->pins(), [](const Pin *pin) {
      return pin->name() == DEVICE::COUT;
    });
    // �ҵ���һ��net
    if (cin_pin != net->pins().end() && cout_pin != net->pins().end()) {

      cin_instance = static_cast<PLCInstance *>(cin_pin->owner());
      cout_instance = static_cast<PLCInstance *>(cout_pin->owner());

      // ����Ϊignord���ڼ���cost�в��ÿ���
      net->set_ignored();
      if (cout_instance->is_fixed() && cin_instance->is_fixed()) {
        // printf("fixed carry chain");
      } else if (!cout_instance->is_fixed() && !cin_instance->is_fixed()) {
        ChainNode *cout_node = create_node(cout_instance);
        ChainNode *cin_node = create_node(cin_instance);
        cout_node->create_to_edge(cin_node);
      } else
        ASSERT(0,
               (CONSOLE::PLC_ERROR % ("illegal constraint for carry chain.")));
      // Ϊnet���ӵ�����slice���������ڵ㣬������һ����
    }
  }
}
/*	���ܣ���find_chains�ҵ���carry chain�ĸ����ڵ㴮����Ӧ��carry
 *chain�����޸�instance������
 *������chains��carry chain�ļ��� ����ֵ��void ˵����
 */
void CarryChainInference::store_chains(CarryChains &chains) {
  for (ChainNodes::value_type &node : _nodes) {
    ChainNode *chain_node = node.second;
    if (!chain_node
             ->_from_edge) { // ���һ����û��from edge����ô��Ϊcarry chain�Ŀ�ʼ
      PLCInstance *owner = chain_node->_owner;
      // hsetΪ������Ҫ��һ�����֣�����Ҫ��carry chain������Ϊcarry
      // chain��ʼ��instance������
      string hset = owner->name();
      //				chains.insert(make_pair(hset,
      // vector<PLCInstance*>())); �õ��������carry chain��vector
      vector<PLCInstance *> &chain = chains[hset];
      // ����carry chain vector
      chain.push_back(owner);
      // ����ѹ�����instance
      while (chain_node->_to_edge) {
        chain.push_back(chain_node->_to_edge->_to_node->_owner);
        chain_node = chain_node->_to_edge->_to_node;
      }
      // ��������������Ǹ�����������
      int rloc = chain.size() - 1;
      Property<string> &hsets =
          create_property<string>(COS::INSTANCE, INSTANCE::HSET);
      Property<Point> &RLOCs =
          create_property<Point>(COS::INSTANCE, INSTANCE::RLOC);
      Property<string> &SET_TYPE =
          create_property<string>(COS::INSTANCE, INSTANCE::SET_TYPE);
      for (PLCInstance *inst : chain) {
        inst->set_property(hsets, hset);
        inst->set_property(RLOCs, Point(rloc--, 0, 0));
        inst->set_property(SET_TYPE, DEVICE::CARRY);
      }
    }
  }
}

} // namespace Place
} // namespace FDU