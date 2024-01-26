#include <string>
#include <map>
#include <vector>
#include <set>
#include <deque>
#include <array>
#include <list>

class Player {
public:

	export_lua int GetId() const;

	uint32_t GetUint32() const;

	export_lua float GetFloat() const;

	std::map<int, int> GetMap() const;

	std::vector<int> GetVector() const;

	std::set<int> GetSet() const;

	std::deque<int> GetDeque() const;

	std::list<int> GetList() const;

private:

	int GetType() const;

public:

	std::string GetName() const;

public:

	export_lua static inline int GetType2();

public:

	void EnterMap(int mapId);

	void EnterMap3(int& mapId);

	void EnterMap1(const int mapId);

	void EnterMap2(int const mapId);
};