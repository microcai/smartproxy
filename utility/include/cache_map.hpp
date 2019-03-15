
#pragma once

#include <tuple>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#define HAVE_CXX14 1
#include <libguarded/shared_guarded.hpp>

struct cache_miss {};

template<typename KeyType, typename ValueType, int cache_aging_time = 30>
class cache_map
{
public:
	ValueType get_cache(const KeyType& key) const
	{
		auto shared_store = m_map.lock_shared();

		auto it = shared_store->find(key);

		if (it == shared_store->end())
		{
			throw cache_miss();
		}

		const std::tuple<ValueType, boost::posix_time::ptime> & value_pack = it->second;

		auto should_be_after = boost::posix_time::second_clock::universal_time() - boost::posix_time::seconds(cache_aging_time);

		if (std::get<1>(value_pack) > should_be_after)
			return std::get<0>(value_pack);
		throw cache_miss();
	}

	void add_to_cache(const KeyType& k , const ValueType& v)
	{
		auto store = m_map.lock();
		store->erase(k);
		store->emplace(k, std::make_tuple(v, boost::posix_time::second_clock::universal_time()));
	}

	void erase(const KeyType& k)
	{
		auto store = m_map.lock();
		store->erase(k);
	}

	void tick()
	{
		auto should_be_after = boost::posix_time::second_clock::universal_time() - boost::posix_time::seconds(30);

		bool do_remove = false;

		{
			auto shared_store = m_map.lock_shared();

			for ( auto && item : * shared_store)
			{
				if (std::get<1>(item.second) < should_be_after)
				{
					do_remove = true;
					break;
				}
			}
		}

		if (!do_remove)
			return;

		auto store = m_map.lock();

		for (auto it = store->begin(); it != store->end(); )
		{
			const std::tuple<ValueType, boost::posix_time::ptime> & value_pack = it->second;

			if (std::get<1>(value_pack) < should_be_after)
			{
				store->erase(it++);
			}
			else
				it++;
		}
	}

	void clear()
	{
		auto store = m_map.lock();
		store->clear();
	}

private:
	libguarded::shared_guarded<
		std::map<KeyType, std::tuple<ValueType, boost::posix_time::ptime>>
	> m_map;
};

template<typename KeyType, typename ValueType>
class cache_map <KeyType, ValueType, 0>
{
public:
	ValueType get_cache(const KeyType& key) const
	{
		auto shared_store = m_map.lock_shared();

		auto it = shared_store->find(key);

		if (it == shared_store->end())
		{
			throw cache_miss();
		}

		return it->second;
	}

	void add_to_cache(const KeyType& k, const ValueType& v)
	{
		auto store = m_map.lock();
		store->erase(k);
		store->emplace(k, v);
	}

	void erase(const KeyType& k)
	{
		auto store = m_map.lock();
		store->erase(k);
	}

	void tick()
	{
	}

	void clear()
	{
		auto store = m_map.lock();
		store->clear();
	}

private:
	libguarded::shared_guarded<
		std::map<KeyType, ValueType>
	> m_map;
};
