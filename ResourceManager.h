#pragma once

#include <future>
#include <mutex>
#include <memory>
#include <utility>
#include <unordered_map>

template<typename Type, typename KeyType, typename ...Args>
class ResourceManager {

	Type(*loadFunc)(Args...);
	void(*freeFunc)(Type* instance);

	std::mutex dataMutex;
	std::unordered_map<KeyType, std::pair<std::shared_ptr<Type>, int>> data;

public:
	ResourceManager(Type(*ploadFunc)(Args...), void(*pfreeFunc)(Type* instance) = [](Type * instance) {}) :
		loadFunc(ploadFunc), freeFunc(pfreeFunc) {}

	~ResourceManager() {

		for (auto it : data)
			freeFunc(&(*it.second.first));
	}

	void loadDataAsync(const KeyType& key, Args... args) {

		auto future = std::async(std::launch::async, [&]() {

				auto pair = std::make_pair(std::make_shared<Type>(loadFunc(std::forward<Args>(args)...)), 0);

				dataMutex.lock();

				data.emplace(key, pair);

				dataMutex.unlock();
			});

		future.get();
	}

	std::shared_ptr<Type> getData(const KeyType& key, Args... args) {

		dataMutex.lock();

		auto it = data.find(key);

		if (it == data.end()) {

			std::shared_ptr<Type> instance = std::make_shared<Type>(loadFunc(std::forward<Args>(args)...));

			data.emplace(key, std::make_pair(instance, 1));

			dataMutex.unlock();
			return instance;
		}
		dataMutex.unlock();

		it->second.second++;
		return it->second.first;
	}

	std::shared_ptr<Type> getData(const KeyType& key) {

		dataMutex.lock();

		auto it = data.find(key);

		if (it == data.end()) {
			dataMutex.unlock();
			return nullptr;
		}

		it->second.second++;

		dataMutex.unlock();
		return it->second.first;
	}

	void freeData(const KeyType& key) {

		dataMutex.lock();

		if (data.size() <= 0) {
			dataMutex.unlock();
			return;
		}

		auto it = data.find(key);

		if (it == data.end()) {
			dataMutex.unlock();
			return;
		}

		if (--it->second.second > 0) {
			dataMutex.unlock();
			return;
		}

		freeFunc(&(*it->second.first));
		data.erase(it);

		dataMutex.unlock();
	}
};