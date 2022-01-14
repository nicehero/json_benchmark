#include <jsoncons/json.hpp>
#include <jsoncons_ext/bson/bson.hpp>
#include <memory>
#include <Windows.h>
#include <iostream>
#include <json/json.hpp>
#include <stdio.h>
#include <fstream>  
#include <set>
#undef max
#undef min
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/pointer.h"
#ifdef WIN32
double MyGetTickCount()
{
	__int64 Freq = 0;
	__int64 Count = 0;
	if (QueryPerformanceFrequency((LARGE_INTEGER*)&Freq)
		&& Freq > 0
		&& QueryPerformanceCounter((LARGE_INTEGER*)&Count))
	{
		return (double)Count / (double)Freq;
	}
	return 0.0;
}
#else
double MyGetTickCount()
{
	return 0.0;
}
#endif

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string EncodeBase58(const unsigned char* pbegin, const unsigned char* pend) {
	// Skip & count leading zeroes.
	int zeroes = 0;
	while (pbegin != pend && *pbegin == 0) {
		pbegin++;
		zeroes++;
	}
	// Allocate enough space in big-endian base58 representation.
	std::vector<unsigned char> b58((pend - pbegin) * 138 / 100 + 1); // log(256) / log(58), rounded up.
	// Process the bytes.
	while (pbegin != pend) {
		int carry = *pbegin;
		// Apply "b58 = b58 * 256 + ch".
		for (std::vector<unsigned char>::reverse_iterator it = b58.rbegin(); it != b58.rend(); it++) {
			carry += 256 * (*it);
			*it = carry % 58;
			carry /= 58;
		}
		assert(carry == 0);
		pbegin++;
	}
	// Skip leading zeroes in base58 result.
	std::vector<unsigned char>::iterator it = b58.begin();
	while (it != b58.end() && *it == 0)
		it++;
	// Translate the result into a string.
	std::string str;
	str.reserve(zeroes + (b58.end() - it));
	str.assign(zeroes, '1');
	while (it != b58.end())
		str += pszBase58[*(it++)];
	return str;
}
template <class T>
std::string writeRapid(T& t) {
	using namespace std;
	using namespace rapidjson;
	StringBuffer  buffer;
	Writer<StringBuffer> writer(buffer);
	t.Accept(writer);
	return buffer.GetString();
}
template <class T,class U,class V>
void addMember(T& t,U& u,const char* s,V v) {
	using namespace std;
	using namespace rapidjson;
	string str = s;
	Value vv(str.c_str(), u.GetAllocator());
	t.AddMember(vv, v, u.GetAllocator());
}
void rapidTest() {
	using namespace std;
	using namespace rapidjson;
	Document j4;
	string jdata4 = R"(
{
"x":"y"
,"o":{"p":"xxx"}
}
)";
	j4.Parse(jdata4.c_str());
	jdata4.clear();

	auto j4c = writeRapid(j4);
	Value j5 = j4.GetObject();
	auto j4cc = writeRapid(j4);
	addMember(j5, j4,"abc",4);
	Value jx(j5, j4.GetAllocator());
	string abc = "abc";
	jx[abc.c_str()] = 66;
	jx["o"]["p"].SetString(abc.c_str(), j4.GetAllocator());
	abc.clear();
	string j5c = writeRapid(j5);
	string jxc = writeRapid(jx);
	j5c = writeRapid(j5);
}

using namespace std;
using namespace jsoncons;
using namespace rapidjson;
using xjson = json;
int main(){
	rapidTest();
	ifstream t("data.txt");
	stringstream ss;
	ss << t.rdbuf();
	string contents(ss.str());
	const char* jdata = contents.c_str();
	Document j3d;
	j3d.Parse(jdata);
	Value j3 = j3d.GetObject();
	nlohmann::json j2 = nlohmann::json::parse(jdata);
	xjson j = xjson::parse(jdata);
	auto dd1 = j.to_string();
	auto dd2 = j2.dump();
	cout <<"root    :" << j.size() << endl;
	cout <<"items   :" << j["items"].size() << endl;
	cout <<"monsters:" << j["monsters"].size() << endl;
	vector<uint8_t> v_bson2 = nlohmann::json::to_bson(j2);
	vector<uint8_t> v_bson;
	bson::encode_bson(j, v_bson);
	cout << "bson size:" << v_bson.size() << endl;
	cout << "bson size:" << v_bson.size() << endl;
	int keyNum = 500;
	vector<string> ks;
	set<string> kss;
	for (int i = 0; i < keyNum; ++i) {
		size_t h = hash<string>{}(to_string(i + 1));
		auto str_hash = EncodeBase58(((const unsigned char*)&h), ((const unsigned char*)&h) + sizeof(h));
		ks.push_back(str_hash);
		kss.insert(str_hash);
	}
	double e = 0.0;
	double s = 0.0;
	double dt = 0.0;
	int repeatCount = 1000;

	{
		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			nlohmann::json j_from_bson = nlohmann::json::from_bson(v_bson);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann bson2json times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			xjson j_from_bson = bson::decode_bson<xjson>(v_bson);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons bson2json times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;
	}

	{
		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			vector<uint8_t> bs = nlohmann::json::to_bson(j2);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann json2bson times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			vector<uint8_t> v_bson;
			jsoncons::bson::encode_bson(j, v_bson);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons json2bson times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;
	}

	{
		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			nlohmann::json jj;
			for (size_t j = 0; j < ks.size(); ++j) {
				jj[ks[j]] = (i + j);
			}
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann insert times:" << keyNum << "x" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(keyNum * repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			xjson jj;
			for (size_t j = 0; j < ks.size(); ++j) {
				jj[ks[j]] = (i + j);
			}
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons insert times:" << keyNum << "x" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(keyNum * repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			Value jj(kObjectType);
			for (size_t j = 0; j < ks.size(); ++j) {
				Value m(ks[j].c_str(), SizeType(ks[j].size()),j3d.GetAllocator());
 				jj.AddMember(m, i + j, j3d.GetAllocator());
			}
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "rapidjson insert times:" << keyNum << "x" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(keyNum * repeatCount) / dt) << endl;
	}

	{
		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			auto& str = ks[i % ks.size()];
			j2[str] = i + 100;
			j2.erase(str);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann insert&erase times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			auto& str = ks[i % ks.size()];
			j[str] = i + 100;
			j.erase(str);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons insert&erase times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			auto& str = ks[i % ks.size()];
			addMember(j3, j3d, str.c_str(), 100);
			j3.RemoveMember(str.c_str());
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "rapidjson insert&erase times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;
	}
	{
		nlohmann::json j2c = j2;
		xjson jc = j;
		Value j3c(j3, j3d.GetAllocator());
		for (auto& s: ks) {
			j2c[s] = 0;
			jc[s] = 0;
			addMember(j3c, j3d, s.c_str(), 0);
		}
		int miss = 10;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			auto& str = ks[i % ks.size()];
			if (i % 100 < miss) {
				j2c.find("miss_key");
			}
			else {
				auto& r = j2c[str];
				r = i + 200;
			}
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann find&replace times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			auto& str = ks[i % ks.size()];
			if (i % 100 < miss) {
				jc.find("miss_key");
			}
			else {
				jc[str] = int64_t(i + 200);
			}
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons find&replace times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			auto& str = ks[i % ks.size()];
			if (i % 100 < miss) {
				j3c.FindMember("miss_key");
			}
			else {
				GenericValue < UTF8<char> > gv(str.c_str(), SizeType(str.size()));
				j3c[str.c_str()] = int64_t(i + 200);
			}
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "rapidjson find&replace times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

	}

	{
		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			nlohmann::json j2cc = j2;
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann copy times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			xjson jcc = j;
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons copy times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			Value j3cc(j3,j3d.GetAllocator());
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "rapidjson copy times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;
	}

	{
		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			nlohmann::json j1p = nlohmann::json::parse(jdata);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann parse times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			xjson j2p = xjson::parse(jdata);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons parse times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			Document j2p;
			j2p.Parse(jdata);
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "rapidjson parse times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;
	}

	{
		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			j2.dump();
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "nlohmann dump times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			j.to_string();
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "jsoncons dump times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;

		s = MyGetTickCount();
		for (int i = 0; i < repeatCount; ++i) {
			StringBuffer  buffer;
			Writer<StringBuffer> writer(buffer);
			j3.Accept(writer);
			buffer.GetString();
		}
		e = MyGetTickCount();
		dt = e - s;
		cout << fixed << "rapidjson dump times:" << repeatCount << " cost:" << dt
			<< " qps:" << uint64_t(double(repeatCount) / dt) << endl;
	}

}


