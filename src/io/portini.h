#ifndef PORTINI_H_
#define PORTINI_H_

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace portini {

namespace internal {

template <typename KeyTy, typename MappedTy, bool Stable>
class Container : public std::unordered_map<KeyTy, MappedTy> {
};

template <typename KeyTy, typename MappedTy>
class Container<KeyTy, MappedTy, true> {
	using ValueTy = std::pair<KeyTy, MappedTy>;
	using BaseTy = std::vector<ValueTy>;

public:
	using const_iterator = typename BaseTy::const_iterator;
	using iterator = typename BaseTy::iterator;
	using size_type = typename BaseTy::size_type;

	const_iterator begin() const {
		return base_.begin();
	}

	iterator begin() {
		return base_.begin();
	}

	const_iterator end() const {
		return base_.end();
	}

	iterator end() {
		return base_.end();
	}

	const_iterator find(const KeyTy& key) const {
		return std::find_if(begin(), end(), [&](const ValueTy& value){ return value.first == key; });
	}

	iterator find(const KeyTy& key) {
		return std::find_if(begin(), end(), [&](const ValueTy& value){ return value.first == key; });
	}

	size_type count(const KeyTy& key) const {
		return find(key) == end() ? 0 : 1;
	}

	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args) {
		auto iter = find_args(args...);
		if (iter != end()) {
			return std::make_pair(iter, false);
		}

		return std::make_pair(base_.emplace(end(), std::forward<Args>(args)...), true);
	}

	size_type erase(const KeyTy& key) {
		auto iter = find(key);
		if (iter == end()) {
			return 0;
		}

		base_.erase(iter);

		return 1;
	}

private:
	template <typename... Args>
	iterator find_args(const KeyTy& key, Args&&...) {
		return find(key);
	}

	BaseTy base_;
};

template <typename Ch, typename Ty>
inline std::basic_string<std::enable_if_t<std::is_same_v<Ch, char>, char>> ToString(Ty src) {
	return std::to_string(src);
}

template <typename Ch, typename Ty>
inline std::basic_string<std::enable_if_t<std::is_same_v<Ch, wchar_t>, wchar_t>> ToString(Ty src) {
	return std::to_wstring(src);
}

} // namespace internal

template <typename Ch>
class GenericKey {
public:
	const std::basic_string<Ch>& GetValue(std::basic_string<Ch>* = nullptr) const {
		return value_;
	}

	template <typename Ty>
	std::enable_if_t<std::is_arithmetic_v<Ty>, Ty> GetValue(Ty* = nullptr) const {
		Ty value{};

		std::basic_istringstream<Ch> iss(value_);
		iss >> value;

		return value;
	}

	template <typename Ty>
	std::enable_if_t<std::is_enum_v<Ty>, Ty> GetValue(Ty* = nullptr) const {
		return static_cast<Ty>(GetValue(static_cast<std::underlying_type_t<Ty>*>(nullptr)));
	}

	void SetValue(const std::basic_string<Ch>& value) {
		value_ = value;
	}

	void SetValue(std::basic_string<Ch>&& value) {
		value_ = std::move(value);
	}

	template <typename Ty>
	std::enable_if_t<std::is_arithmetic_v<Ty>> SetValue(Ty value) {
		value_ = internal::ToString<Ch>(value);
	}

	template <typename Ty>
	std::enable_if_t<std::is_enum_v<Ty>> SetValue(Ty value) {
		SetValue(static_cast<std::underlying_type_t<Ty>>(value));
	}

	template <typename Ty>
	operator Ty() const {
		return GetValue(static_cast<Ty*>(nullptr));
	}

	template <typename Ty>
	GenericKey& operator =(Ty&& value) {
		SetValue(std::forward<Ty>(value));
		return *this;
	}

private:
	std::basic_string<Ch> value_;
};

using Key = GenericKey<char>;
using KeyW = GenericKey<wchar_t>;

template <typename Ch, bool Stable>
class GenericSection {
	using Container = internal::Container<std::basic_string<Ch>, GenericKey<Ch>, Stable>;

public:
	using ConstIterator = typename Container::const_iterator;
	using Iterator = typename Container::iterator;

	GenericKey<Ch>& CreateKey(const std::basic_string<Ch>& name) {
		return keys_.emplace(name, GenericKey<Ch>()).first->second;
	}

	GenericKey<Ch>& CreateKey(std::basic_string<Ch>&& name) {
		return keys_.emplace(std::move(name), GenericKey<Ch>()).first->second;
	}

	void DeleteKey(const std::basic_string<Ch>& name) {
		keys_.erase(name);
	}

	bool HasKey(const std::basic_string<Ch>& name) const {
		return keys_.count(name) == 1;
	}

	const GenericKey<Ch>& GetKey(const std::basic_string<Ch>& name) const {
		auto iter = keys_.find(name);
		if (iter == keys_.end()) {
			throw std::logic_error("KEY NOT FOUND");
		}

		return iter->second;
	}

	GenericKey<Ch>& GetKey(const std::basic_string<Ch>& name) {
		auto iter = keys_.find(name);
		if (iter == keys_.end()) {
			throw std::logic_error("KEY NOT FOUND");
		}

		return iter->second;
	}

	const GenericKey<Ch>& operator [](const std::basic_string<Ch>& name) const {
		return GetKey(name);
	}

	GenericKey<Ch>& operator [](const std::basic_string<Ch>& name) {
		return GetKey(name);
	}

	ConstIterator begin() const {
		return keys_.begin();
	}

	Iterator begin() {
		return keys_.begin();
	}

	ConstIterator end() const {
		return keys_.end();
	}

	Iterator end() {
		return keys_.end();
	}

private:
	Container keys_;
};

using Section = GenericSection<char, false>;
using SectionW = GenericSection<wchar_t, false>;
using StableSection = GenericSection<char, true>;
using StableSectionW = GenericSection<wchar_t, true>;

template <typename Ch, bool Stable>
class GenericDocument {
	using Container = internal::Container<std::basic_string<Ch>, GenericSection<Ch, Stable>, Stable>;

public:
	using ConstIterator = typename Container::const_iterator;
	using Iterator = typename Container::iterator;

	bool ParseFromFile(const Ch* filename) {
		std::basic_ifstream<Ch> ifs(filename);
		if (!ifs.is_open()) {
			return false;
		}

		bool ret = ParseFromStream(ifs);
		ifs.close();

		return ret;
	}

	bool ParseFromString(const std::basic_string<Ch>& str) {
		std::basic_istringstream<Ch> iss(str);
		return ParseFromStream(iss);
	}

	bool SerializeToFile(const Ch* filename) const {
		std::basic_ofstream<Ch> ofs(filename, std::ios_base::binary);
		if (!ofs.is_open()) {
			return false;
		}

		SerializeToStream(ofs);
		ofs.close();

		return true;
	}

	std::basic_string<Ch> SerializeToString() const {
		std::basic_ostringstream<Ch> oss;
		SerializeToStream(oss);
		return oss.str();
	}

	GenericSection<Ch, Stable>& CreateSection(const std::basic_string<Ch>& name) {
		return sections_.emplace(name, GenericSection<Ch, Stable>()).first->second;
	}

	GenericSection<Ch, Stable>& CreateSection(std::basic_string<Ch>&& name) {
		return sections_.emplace(std::move(name), GenericSection<Ch, Stable>()).first->second;
	}

	void DeleteSection(const std::basic_string<Ch>& name) {
		sections_.erase(name);
	}

	bool HasSection(const std::basic_string<Ch>& name) const {
		return sections_.count(name) == 1;
	}

	const GenericSection<Ch, Stable>& GetSection(const std::basic_string<Ch>& name) const {
		auto iter = sections_.find(name);
		if (iter == sections_.end()) {
			throw std::logic_error("SECTION NOT FOUND");
		}

		return iter->second;
	}

	GenericSection<Ch, Stable>& GetSection(const std::basic_string<Ch>& name) {
		auto iter = sections_.find(name);
		if (iter == sections_.end()) {
			throw std::logic_error("SECTION NOT FOUND");
		}

		return iter->second;
	}

	const GenericSection<Ch, Stable>& operator [](const std::basic_string<Ch>& name) const {
		return GetSection(name);
	}

	GenericSection<Ch, Stable>& operator [](const std::basic_string<Ch>& name) {
		return GetSection(name);
	}

	ConstIterator begin() const {
		return sections_.begin();
	}

	Iterator begin() {
		return sections_.begin();
	}

	ConstIterator end() const {
		return sections_.end();
	}

	Iterator end() {
		return sections_.end();
	}

private:
	bool ParseFromStream(std::basic_istream<Ch>& is) {
		GenericSection<Ch, Stable>* ctx = nullptr;

		do {
			std::basic_string<Ch> line;

			std::getline(is, line);
			if (line.empty()) {
				continue;
			}

			if (!Parse(line, &ctx)) {
				return false;
			}
		} while (!is.eof());

		return true;
	}

	bool Parse(const std::basic_string<Ch>& line, GenericSection<Ch, Stable>** ctx) {
		Ch ch = *line.begin();

		if (ch == ';' || ch == '#') {
			return true;
		} else if (ch == '[') {
			if (*line.rbegin() != ']') {
				return false;
			}

			*ctx = &CreateSection(line.substr(1, line.length() - 2));

			return true;
		} else {
			auto pos = line.find('=');
			if (pos == std::basic_string<Ch>::npos) {
				return false;
			}

			(*ctx)->CreateKey(line.substr(0, pos)).SetValue(line.substr(pos + 1));

			return true;
		}
	}

	void SerializeToStream(std::basic_ostream<Ch>& os) const {
		for (auto& section : *this) {
			os << '[' << section.first << ']' << std::endl;

			for (auto& key : section.second) {
				os << key.first << '=' << key.second.GetValue() << std::endl;
			}
		}
	}

	Container sections_;
};

using Document = GenericDocument<char, false>;
using DocumentW = GenericDocument<wchar_t, false>;
using StableDocument = GenericDocument<char, true>;
using StableDocumentW = GenericDocument<wchar_t, true>;

} // namespace portini

#endif // PORTINI_H_
