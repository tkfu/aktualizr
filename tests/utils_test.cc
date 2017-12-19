#include <gtest/gtest.h>

#include <fstream>
#include <set>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_smallint.hpp>

#include "utils.h"

bool CharOk(char c) {
  if (('a' <= c) && (c <= 'z')) {
    return true;
  } else if (('0' <= c) && (c <= '9')) {
    return true;
  } else if (c == '-') {
    return true;
  } else {
    return false;
  }
}

bool PrettyNameOk(const std::string &name) {
  if (name.size() < 5) {
    return false;
  }
  for (std::string::const_iterator c = name.begin(); c != name.end(); ++c) {
    if (!CharOk(*c)) {
      return false;
    }
  }
  return true;
}

TEST(Utils, PrettyNameOk) {
  EXPECT_TRUE(PrettyNameOk("foo-bar-123"));
  EXPECT_FALSE(PrettyNameOk("NoCapitals"));
  EXPECT_FALSE(PrettyNameOk(""));
  EXPECT_FALSE(PrettyNameOk("foo-bar-123&"));
}

TEST(Utils, getHostname) { EXPECT_NE(Utils::getHostname(), ""); }

/**
 * \verify{\tst{144}} Check that aktualizr can generate a pet name
 */
TEST(Utils, GenPrettyNameSane) {
  std::set<std::string> names;
  for (int i = 0; i < 100; i++) {
    std::string name = Utils::genPrettyName();
    EXPECT_EQ(0, names.count(name));
    names.insert(name);
    EXPECT_EQ(1, names.count(name));
  }
}

TEST(Utils, RandomUuidSane) {
  std::set<std::string> uuids;
  for (int i = 0; i < 1000; i++) {
    std::string uuid = Utils::randomUuid();
    EXPECT_EQ(0, uuids.count(uuid));
    uuids.insert(uuid);
    EXPECT_EQ(1, uuids.count(uuid));
  }
}

TEST(Utils, FromBase64) {
  // Generated using python's base64.b64encode
  EXPECT_EQ("aGVsbG8=", Utils::toBase64("hello"));
  EXPECT_EQ("", Utils::toBase64(""));
  EXPECT_EQ("CQ==", Utils::toBase64("\t"));
  EXPECT_EQ("YWI=", Utils::toBase64("ab"));
  EXPECT_EQ("YWJj", Utils::toBase64("abc"));
}

TEST(Utils, Base64RoundTrip) {
  boost::mt19937 gen;
  boost::random::uniform_smallint<char> chars(std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

  boost::random::uniform_smallint<int> length(0, 20);

  for (int test = 0; test < 100; test++) {
    int len = length(gen);
    std::string original;
    for (int i = 0; i < len; i++) {
      original += chars(gen);
    }
    std::string b64 = Utils::toBase64(original);
    std::string output = Utils::fromBase64(b64);
    EXPECT_EQ(original, output);
  }
}

TEST(Utils, TemporaryFile) {
  boost::filesystem::path p;
  {
    TemporaryFile f("ahint");
    p = f.Path();
    EXPECT_FALSE(boost::filesystem::exists(p));  // The file shouldn't already exist
    std::ofstream file(p.c_str());
    file << "test";
    file.close();
    EXPECT_TRUE(file);                                       // The write succeeded
    EXPECT_TRUE(boost::filesystem::exists(p));               // The file should exist here
    EXPECT_NE(p.string().find("ahint"), std::string::npos);  // The hint is included in the filename
  }
  EXPECT_FALSE(boost::filesystem::exists(p));  // The file gets deleted by the RAII dtor
}

TEST(Utils, TemporaryFilePutContents) {
  TemporaryFile f("ahint");
  f.PutContents("thecontents");
  EXPECT_TRUE(boost::filesystem::exists(f.Path()));
  std::ifstream a(f.Path().c_str());
  std::string b;
  a >> b;
  EXPECT_EQ(b, "thecontents");
}

TEST(Utils, copyDir) {
  boost::filesystem::remove_all("tests/test_data/test_copy_dir");

  Utils::writeFile("tests/test_data/test_copy_dir/from/1/foo", std::string("foo"));
  Utils::writeFile("tests/test_data/test_copy_dir/from/1/2/bar", std::string("bar"));
  Utils::writeFile("tests/test_data/test_copy_dir/from/1/2/baz", std::string("baz"));

  Utils::copyDir("tests/test_data/test_copy_dir/from", "tests/test_data/test_copy_dir/to");
  EXPECT_TRUE(boost::filesystem::exists("tests/test_data/test_copy_dir/to"));
  EXPECT_TRUE(boost::filesystem::exists("tests/test_data/test_copy_dir/to/1"));
  EXPECT_TRUE(boost::filesystem::exists("tests/test_data/test_copy_dir/to/1/foo"));
  EXPECT_TRUE(boost::filesystem::exists("tests/test_data/test_copy_dir/to/1/2"));
  EXPECT_TRUE(boost::filesystem::exists("tests/test_data/test_copy_dir/to/1/2/bar"));
  EXPECT_TRUE(boost::filesystem::exists("tests/test_data/test_copy_dir/to/1/2/baz"));
  EXPECT_EQ(Utils::readFile("tests/test_data/test_copy_dir/to/1/foo"), "foo");
  EXPECT_EQ(Utils::readFile("tests/test_data/test_copy_dir/to/1/2/bar"), "bar");
  EXPECT_EQ(Utils::readFile("tests/test_data/test_copy_dir/to/1/2/baz"), "baz");

  boost::filesystem::remove_all("tests/test_data/test_copy_dir");
}

TEST(Utils, writeFileWithoutDirAutoCreation) {
  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");

  boost::filesystem::create_directories("tests/test_data/test_write_file_dir/1/2");
  Utils::writeFile("tests/test_data/test_write_file_dir/1/foo", std::string("foo"), false);
  Utils::writeFile("tests/test_data/test_write_file_dir/1/2/bar", std::string("bar"), false);

  EXPECT_EQ(Utils::readFile("tests/test_data/test_write_file_dir/1/foo"), "foo");
  EXPECT_EQ(Utils::readFile("tests/test_data/test_write_file_dir/1/2/bar"), "bar");

  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");
}

TEST(Utils, writeFileWithDirAutoCreation) {
  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");

  Utils::writeFile("tests/test_data/test_write_file_dir/1/foo", std::string("foo"), true);
  Utils::writeFile("tests/test_data/test_write_file_dir/1/2/bar", std::string("bar"), true);

  EXPECT_EQ(Utils::readFile("tests/test_data/test_write_file_dir/1/foo"), "foo");
  EXPECT_EQ(Utils::readFile("tests/test_data/test_write_file_dir/1/2/bar"), "bar");

  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");
}

TEST(Utils, writeFileWithDirAutoCreationDefault) {
  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");

  Utils::writeFile("tests/test_data/test_write_file_dir/1/foo", std::string("foo"));
  Utils::writeFile("tests/test_data/test_write_file_dir/1/2/bar", std::string("bar"));

  EXPECT_EQ(Utils::readFile("tests/test_data/test_write_file_dir/1/foo"), "foo");
  EXPECT_EQ(Utils::readFile("tests/test_data/test_write_file_dir/1/2/bar"), "bar");

  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");
}

TEST(Utils, writeFileWithoutDirAutoCreationException) {
  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");

  try {
    Utils::writeFile("tests/test_data/test_write_file_dir/1/foo", std::string("foo"), false);
  } catch (...) {
    boost::filesystem::remove_all("tests/test_data/test_write_file_dir");
    return;
  }
  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");
}

TEST(Utils, writeFileJson) {
  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");

  Json::Value val;
  val["key"] = "val";

  Utils::writeFile("tests/test_data/test_write_file_dir/1/foo", val);
  Json::Value result_json = Utils::parseJSONFile("tests/test_data/test_write_file_dir/1/foo");
  EXPECT_EQ(result_json["key"].asString(), val["key"].asString());

  boost::filesystem::remove_all("tests/test_data/test_write_file_dir");
}

TEST(Utils, hex2bin) {
  unsigned char bin[3];

  Utils::hex2bin("01aeb4", bin);
  EXPECT_EQ(bin[0], 0x01);
  EXPECT_EQ(bin[1], 0xae);
  EXPECT_EQ(bin[2], 0xb4);
}

TEST(Utils, intToString) {
  unsigned int a = 456;
  EXPECT_EQ(Utils::intToString(a), "456");
}

TEST(Utils, shell) {
  std::string out;
  int statuscode = Utils::shell("ls /", &out);
  EXPECT_EQ(statuscode, 0);

  statuscode = Utils::shell("ls /nonexistentdir123", &out);
  EXPECT_NE(statuscode, 0);
}

#ifndef __NO_MAIN__
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
