#include "plc_app.h"

using namespace FDU::Place;
/************************************************************************/
/*	���ܣ��ж��Ƿ���Խ�����������Խ�������ture�����򷵻�false
 *	������void
 *	����ֵ��int
 *	˵������ں���
 */
/************************************************************************/
int main(int argc, char *argv[]) {
  PlaceApp app;
  // ��������
  app.parse_command(argc, argv);
  // ��ʼ����
  app.try_process();

  return EXIT_SUCCESS;
}