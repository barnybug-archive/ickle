#include "Xml.h"

int main() {
  string text("<sms_message><source>MTN</source><destination_UIN>68065772</destination_UIN><sender>447811241712</sender><senders_network> </senders_network><text>Hello, me &amp; my mobile &lt;tag&gt;.</text><time><another>  a </another></time></sms_message>");
  string::iterator s = text.begin();
  XmlNode *top = XmlNode::parse(s, text.end());
  if (top != NULL) {
    cout << top->toString(0) << endl;
    delete top;
  }

  XmlBranch con("icq_sms_message");
  con.pushnode(new XmlLeaf("destination","+447811241712"));
  con.pushnode(new XmlLeaf("text","<test & test>"));
  con.pushnode(new XmlLeaf("codepage","1252"));
  con.pushnode(new XmlLeaf("senders_UIN","11715585"));
  con.pushnode(new XmlLeaf("senders_name","tom"));
  con.pushnode(new XmlLeaf("delivery_receipt","Yes"));
  con.pushnode(new XmlLeaf("time","boo"));

  cout << con.toString(0) << endl;

}
