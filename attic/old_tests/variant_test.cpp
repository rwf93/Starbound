#include <fstream>

#include "StarString.hpp"
#include "StarFile.hpp"
#include "StarVariant.hpp"

using namespace Star;
using namespace std;

int main(int argc, char** argv) {
  try {
    String s1("This is a [日本語] Unicode String");

    Variant v = Variant::parse("{ \"first\" : \"日本語\", \"second\" : \"foobar\\u02b0\" }");
    cout << "Parsed variant: " << v.repr() << endl;

    VariantMap vm;
    vm["one"] = true;
    vm["two"] = 22222222222222222;
    vm["three"] = 3.3;
    vm["four"] = "日本語";
    vm["largeInt"] = 1234567890123456789L;

    String s2 = Variant(vm).repr();
    cout << "Map to json: " << s2 << endl;
    cout << "json to map: " << Variant::parse(s2) << endl;

    VariantList vl(4, Variant());
    vl[0] = "foo";
    vl[1] = 22222222222222222;
    vl[2] = 3.3;
    vl[3] = "日本語 bar";

    String s3 = Variant(vl).repr();
    cout << "List to json: " << s3 << endl;
    cout << "json to list: " << Variant::parse(s3) << endl;

    vm["five"] = vl;

    cout << "Write to file: ./jsontest.txt" << endl;
    ofstream ofile("./jsontest.txt");
    Variant(vm).writeJson(ofile, true);
    ofile << endl;
    ofile.close();

    cout << "Read from file: ./jsontest.txt" << endl;
    ifstream ifile("./jsontest.txt");
    cout << Variant(Variant::readJson(ifile)).repr(2) << endl;
    ifile.close();

    cout << "Write binary to file: ./jsontest.bin" << endl;
    DataStream dataOs(File::open("./jsontest.bin", IODevice::Write));
    dataOs << Variant(vm);
    dataOs.device().close();

    cout << "Read binary from file: ./jsontest.bin" << endl;
    dataOs.device().open(IODevice::Read);
    cout << dataOs.read<Variant>().repr(2) << endl;
    dataOs.device().close();

    cout << "Write binary to memory" << endl;
    DataStreamBuffer dataBuffer;
    dataBuffer << Variant(vm);
    auto memData = dataBuffer.data();

    cout << "Read binary from memory" << endl;
    dataBuffer.reset(memData);
    cout << dataBuffer.read<Variant>().repr(2) << endl;

    Variant queryTest = Variant::parse(R"JSON(
        {"foo" : [{}, {"bar" : {"baz" : [0, 1, 2] }}]}
      )JSON");
    
    coutf("query result: %s\n", queryTest.query("foo[1].bar.baz[2]"));
    try {
      queryTest.queryMap("foo[1].bar.baz[2]");
    } catch (VariantException const& e) {
      coutf("Expected exception caught: %s\n", e.message());
    }

    Variant v1 = VariantMap();
    v1["foo"] = 88;
    v1["bar"] = 1.0;
    v1["baz"] = VariantList{1, 2, 3};

    Variant v2 = v1;
    v2["bar"] = Variant();

    coutf("Variant v1 shoudl be different from variant v2 (checking CopyOnWrite)\n%s\n%s\n", v1.repr(2), v2.repr(2));

    Variant v3 = VariantMap();
    v3["foo"] = 88;
    v3["bar"] = 1.0;
    v3["baz"] = VariantList{1, 2, 3};

    coutf("Variant v1 shoudl be identical to v3, though have different data: is %s\n", v1 == v3);

  } catch (std::exception const& e) {
    coutf("exception caught: %s\n", e.what());
    return 1;
  }

  return 0;
}
