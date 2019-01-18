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
#define HAVE_IRCD_CTX_PARALLEL_H

namespace ircd::ctx
{
	template<class arg> class parallel;
}

template<class arg>
struct ircd::ctx::parallel
{
	using closure = std::function<void (arg &)>;

	pool *p {nullptr};
	vector_view<arg> a;
	closure c;
	dock d;
	std::exception_ptr eptr;
	ushort snd {0};
	ushort rcv {0};
	ushort out {0};

	void rethrow_any_exception();
	void receiver() noexcept;
	void sender() noexcept;

  public:
	void wait_avail();
	void wait_done();

	void operator()();
	void operator()(const arg &a);

	parallel(pool &, const vector_view<arg> &, closure);
	parallel(parallel &&) = delete;
	parallel(const parallel &) = delete;
	~parallel() noexcept;
};

template<class arg>
ircd::ctx::parallel<arg>::parallel(pool &p,
                                   const vector_view<arg> &a,
                                   closure c)
:p{&p}
,a{a}
,c{std::move(c)}
{
	p.min(this->a.size());
}

template<class arg>
ircd::ctx::parallel<arg>::~parallel()
noexcept
{
	const uninterruptible::nothrow ui;
	wait_done();
}

template<class arg>
void
ircd::ctx::parallel<arg>::operator()(const arg &a)
{
	rethrow_any_exception();
	assert(snd < this->a.size());
	this->a.at(snd) = a;
	sender();
	wait_avail();
	assert(out < this->a.size());
}

template<class arg>
void
ircd::ctx::parallel<arg>::operator()()
{
	rethrow_any_exception();
	sender();
	wait_avail();
	assert(out < this->a.size());
}

template<class arg>
void
ircd::ctx::parallel<arg>::sender()
noexcept
{
	++snd;
	snd %= this->a.size();

	++out;
	assert(out <= this->a.size());

	auto &p(*this->p);
	auto func
	{
		std::bind(&parallel::receiver, this)
	};

	if(likely(p.size()))
		p(std::move(func));
	else
		func();
}

template<class arg>
void
ircd::ctx::parallel<arg>::receiver()
noexcept
{
	auto &a{this->a.at(rcv % this->a.size())};
	++rcv;

	if(!this->eptr) try
	{
		c(a);
	}
	catch(...)
	{
		this->eptr = std::current_exception();
	}

	assert(out <= this->a.size());
	assert(out > 0);
	--out;
	d.notify_one();
}

template<class arg>
void
ircd::ctx::parallel<arg>::rethrow_any_exception()
{
	if(likely(!this->eptr))
		return;

	wait_done();
	const auto eptr(this->eptr);
	this->eptr = {};
	std::rethrow_exception(eptr);
}

template<class arg>
void
ircd::ctx::parallel<arg>::wait_avail()
{
	d.wait([this]
	{
		return out < a.size();
	});
}

template<class arg>
void
ircd::ctx::parallel<arg>::wait_done()
{
	d.wait([this]
	{
		return !out;
	});
}
