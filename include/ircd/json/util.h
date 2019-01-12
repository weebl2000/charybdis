// Matrix Construct
//
// Copyright (C) Matrix Construct Developers, Authors & Contributors
// Copyright (C) 2016-2018 Jason Volk <jason@zemos.net>
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice is present in all copies. The
// full license for this software is available in the LICENSE file.

#pragma once
#define HAVE_IRCD_JSON_UTIL_H

namespace ircd::json
{
	using name_hash_t = size_t;
	constexpr name_hash_t name_hash(const string_view name);
	constexpr name_hash_t operator ""_(const char *const name, const size_t len);

	extern const string_view literal_null;
	extern const string_view literal_true;
	extern const string_view literal_false;
	extern const string_view empty_string;
	extern const string_view empty_object;
	extern const string_view empty_array;
	extern const int64_t undefined_number;

	size_t serialized(const string_view &);
	string_view stringify(mutable_buffer &, const string_view &);

	using members = std::initializer_list<member>;

	// Validate JSON - checks if valid JSON (not canonical).
	bool valid(const string_view &, std::nothrow_t) noexcept;
	void valid(const string_view &);
	std::string why(const string_view &);

	// (Internal) validates output
	void valid_output(const string_view &, const size_t &expected);
}

constexpr ircd::json::name_hash_t
ircd::json::operator ""_(const char *const text, const size_t len)
{
	return name_hash(string_view(text, len));
}

constexpr ircd::json::name_hash_t
ircd::json::name_hash(const string_view name)
{
	return ircd::hash(name);
}
