/*
* Copyright (c) 2015 Cryptonomex, Inc., and contributors.
* Copyright (c) 2018- ¦ÌNEST Foundation, and contributors.
*
* The MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <graphene/chain/watch_dog_evaluator.hpp>
#include <graphene/chain/protocol/watch_dog.hpp>
#include <graphene/chain/watch_dog_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/protocol/asset.hpp>

#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/database.hpp>
#include <fc/time.hpp>

namespace graphene { namespace chain {

	template <typename OP_TYPE>
	void change_watch_dog_state(database& d, const OP_TYPE& op, watch_dog_object::watch_state s)
	{
		auto wd = d.get_watch_dog_by_account(op.account);
		//FC_ASSERT(wd != nullptr, "watch dog is not founded");
		d.modify<watch_dog_object>(*wd, [&](watch_dog_object& obj)
		{
			obj.state = s;
		});
	}

	template <>
	void change_watch_dog_state<remind_account_operation>
		(database& d, const remind_account_operation& op, watch_dog_object::watch_state s)
	{
		auto wd = d.get_watch_dog_by_account(op.account);
		//FC_ASSERT(wd != nullptr, "watch dog is not founded");
		d.modify<watch_dog_object>(*wd, [&](watch_dog_object& obj)
		{
			obj.state = s;
			obj.question_send_time = fc::time_point::now();
		});
	}

	template <>
	void change_watch_dog_state<start_account_recover_operation>
		(database& d, const start_account_recover_operation& op, watch_dog_object::watch_state s)
	{
		auto wd = d.get_watch_dog_by_account(op.account);
		//FC_ASSERT(wd != nullptr, "watch dog is not founded");
		d.modify<watch_dog_object>(*wd, [&](watch_dog_object& obj)
		{
			obj.state = s;
			obj.recover_begin_time = fc::time_point::now();
		});
	}

	//====================================================================================//

	void_result create_watch_dog_evaluator::do_evaluate(const operation_type& op)
	{ try {
		database& d = db();
		auto account_name = d.get_account_stats_by_owner(op.watch_account).name;
		bool have_watch_dog = d.check_account_have_watch_dog(op.watch_account);

		FC_ASSERT(!have_watch_dog, "the account ${a} already has a watch dog", ("a", account_name));
		FC_ASSERT(op.watch_account != op.inherit_account, "watch account and inherit account should not be the same one");

		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	object_id_type create_watch_dog_evaluator::do_apply(const operation_type& op)
	{ try {
		database& d = db();
		const auto& wd_obj = d.create<watch_dog_object>([&op](watch_dog_object& obj)
		{
			obj.state = watch_dog_object::idle;
			obj.watch_account = op.watch_account;
			obj.inherit_account = op.inherit_account;
			obj.answer_hash = op.answer_hash;
		});
		return wd_obj.id;
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result prepare_watch_dog_evaluator::do_evaluate(const operation_type& op)
	{
		return void_result();
	}

	void_result prepare_watch_dog_evaluator::do_apply(const operation_type& op)
	{ try {
		database& d = db();
		change_watch_dog_state(d, op, watch_dog_object::ready);
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result remind_account_evaluator::do_evaluate(const operation_type& op)
	{
		return void_result();
	}

	void_result remind_account_evaluator::do_apply(const operation_type& op)
	{ try {
		database& d = db();
		change_watch_dog_state(d, op, watch_dog_object::question_sended);
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result answer_watch_dog_evaluator::do_evaluate(const operation_type& op)
	{ try {
		database& d = db();

		bool have_watch_dog = d.check_account_have_watch_dog(op.account);
		FC_ASSERT(have_watch_dog, "the watch dog is not found");

		auto wd = d.get_watch_dog_by_account(op.account);
		FC_ASSERT(op.answer_hash == wd->answer_hash, "the answer is not right");
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result answer_watch_dog_evaluator::do_apply(const operation_type& op)
	{ try {
		database& d = db();
		change_watch_dog_state(d, op, watch_dog_object::answer_received);
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result start_account_recover_evaluator::do_evaluate(const operation_type& op)
	{
		return void_result();
	}

	void_result start_account_recover_evaluator::do_apply(const operation_type& op)
	{ try {
		database& d = db();
		change_watch_dog_state(d, op, watch_dog_object::recover_begin);
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result transfer_lost_asset_evaluator::do_evaluate(const operation_type& op)
	{
		return void_result();
	}

	void_result transfer_lost_asset_evaluator::do_apply(const operation_type& op)
	{ try {
		database& d = db();

		auto wd = d.get_watch_dog_by_account(op.account);
		//FC_ASSERT(wd != nullptr, "watch dog is not founded");

		vector<asset> asset_vec;
		const account_balance_index& balance_index = d.get_index_type<account_balance_index>();
		auto range = balance_index.indices().get<by_account_asset>().equal_range(boost::make_tuple(wd->watch_account));
		for (const account_balance_object& balance : boost::make_iterator_range(range.first, range.second))
			asset_vec.push_back(asset(balance.get_balance()));

		for (const auto& a : asset_vec)
		{
			d.adjust_balance(wd->watch_account, -a);
			d.adjust_balance(wd->inherit_account, a);
		}

		d.modify<watch_dog_object>(*wd, [&](watch_dog_object& obj)
		{
			obj.state = watch_dog_object::recover_end;
		});

		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result delete_watch_dog_evaluator::do_evaluate(const operation_type& op)
	{
		return void_result();
	}

	void_result delete_watch_dog_evaluator::do_apply(const operation_type& op)
	{ try {
		database& d = db();
		auto watch_dog_id = d.get_watch_dog_id_by_account(op.account);
		d.remove(watch_dog_id);
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

} }
