/*
* Copyright (c) 2015 Cryptonomex, Inc., and contributors.
* Copyright (c) 2018- μNEST Foundation, and contributors.
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

#include <graphene/chain/htlc_evaluator.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/htlc_object.hpp>
#include <fc/crypto/sha1.hpp>

namespace graphene { namespace chain {

	void_result htlc_prepare_evaluator::do_evaluate(const htlc_prepare_operation& op)
	{
	try {
		const database& d = db();
		const account_object& depositor  = op.depositor(d);
		const asset_object&   asset_type = op.amount.asset_id(d);
		
		try {
			GRAPHENE_ASSERT(
				is_authorized_asset(d, depositor, asset_type),
				transfer_from_account_not_whitelisted,
				"'depositor' account ${from} is not whitelisted for asset ${asset}",
				("depositor", op.depositor)
				("asset", op.amount.asset_id)
				);

			bool insufficient_balance = d.get_balance(depositor, asset_type).amount >= op.amount.amount + op.fee.amount;
			FC_ASSERT(insufficient_balance,
				"Insufficient Balance: ${balance}, account '${a}' unable to create htlc with '${amount}'",
				("a", depositor.name)("amount", d.to_pretty_string(op.amount))("balance", d.to_pretty_string(d.get_balance(depositor, asset_type))));

			// Validate: recipient account exists

			return void_result();
		} FC_RETHROW_EXCEPTIONS(error, "Unable to create htlc");

	} FC_CAPTURE_AND_RETHROW((op)) }

	object_id_type htlc_prepare_evaluator::do_apply(const htlc_prepare_operation& op)
	{
	try {
		database& d = db();
		//d.adjust_balance(op.depositor, -op.fee);
		d.adjust_balance(op.depositor, -op.amount);

		const auto& new_htlc_obj = d.create<htlc_object>([&op](htlc_object& obj)
		{
			obj.depositor      = op.depositor;
			obj.recipient      = op.recipient;
			obj.amount         = op.amount;
			obj.expiration     = op.timeout_threshold;
			obj.pending_fee    = op.fee;
			obj.hash_algorithm = op.hash_algorithm;
			obj.preimage_hash  = op.preimage_hash;
			obj.preimage_size  = op.preimage_length;
		});
		return new_htlc_obj.id;

	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result htlc_redeem_evaluator::do_evaluate(const htlc_redeem_operation& op)
	{
	try {
		const database& d = db();
		const account_object& fee_paying_account = op.fee_paying_account(d);
		const asset_object&   asset_type = op.fee.asset_id(d);
		const htlc_object& htlc_obj = d.get_htlc(op.htlc_id);

		try {
			GRAPHENE_ASSERT(
				is_authorized_asset(d, fee_paying_account, asset_type),
				transfer_from_account_not_whitelisted,
				"'fee_paying_account' account ${from} is not whitelisted for asset ${asset}",
				("fee_paying_account", op.fee_paying_account)
				("asset", op.fee.asset_id)
				);

			bool insufficient_balance = d.get_balance(fee_paying_account, asset_type).amount >= op.fee.amount;
			FC_ASSERT(insufficient_balance, "insufficient balance to redeem htlc");

			FC_ASSERT(fc::time_point::now() < htlc_obj.expiration, "timeout exceeded");
			FC_ASSERT(op.preimage.size() == htlc_obj.preimage_size, "preimage length mismatch");

			if ("sha256" == htlc_obj.hash_algorithm)
				check_preimage_hash<fc::sha256>(op, htlc_obj);
			else if ("sha1" == htlc_obj.hash_algorithm)
				check_preimage_hash<fc::sha1>(op, htlc_obj);
			else if ("ripemd160" == htlc_obj.hash_algorithm)
				check_preimage_hash<fc::ripemd160>(op, htlc_obj);
			else
				FC_ASSERT(false, "unsupported hash algorithm");

			return void_result();
		} FC_RETHROW_EXCEPTIONS(error, "Unable to redeem htlc");

	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result htlc_redeem_evaluator::do_apply(const htlc_redeem_operation& op)
	{
	try {
		database& d = db();
		const htlc_object& htlc_obj = d.get_htlc(op.htlc_id);

		//d.adjust_balance(op.fee_paying_account, -op.fee);
		d.adjust_balance(htlc_obj.recipient, htlc_obj.amount);

		d.modify<htlc_object>(htlc_obj, [&op](htlc_object& obj)
		{
			obj.preimage_tx_id; //已经上链的区块号
		});
		d.remove(op.htlc_id);

		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result htlc_extend_expiry_evaluator::do_evaluate(const htlc_extend_expiry_operation& op)
	{ try {
		const database& d = db();
		const account_object& depositor  = op.depositor(d);
		const asset_object&   asset_type = op.fee.asset_id(d);

		const htlc_object& htlc_obj = d.get_htlc(op.htlc_id);

		try {
			FC_ASSERT(op.depositor == htlc_obj.depositor,
				"account ${a} is not the depositor of htlc ${h}",
				("a", depositor.name)("h", op.htlc_id));

			bool insufficient_balance = d.get_balance(depositor, asset_type).amount >= op.fee.amount;
			FC_ASSERT(insufficient_balance,
				"Insufficient balance: ${balance}, '${a}' unable to pay the fee to extend htlc expire.",
				("balance", d.to_pretty_string(d.get_balance(depositor, asset_type))) ("a", depositor.name));

		} FC_RETHROW_EXCEPTIONS(error, "Unable to extend expire of htlc");

		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result htlc_extend_expiry_evaluator::do_apply(const htlc_extend_expiry_operation& op)
	{ try {
		database& d = db();
		//d.adjust_balance(op.depositor, -op.fee);
		const htlc_object& htlc_obj = d.get_htlc(op.htlc_id);
		d.modify<htlc_object>(htlc_obj, [&op](htlc_object& obj)
		{
			obj.expiration = op.timeout_threshold;
		});
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result htlc_refund_evaluator::do_evaluate(const htlc_refund_operation& op)
	{ try {
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

	void_result htlc_refund_evaluator::do_apply(const htlc_refund_operation& op)
	{ try {
		database& d = db();
		const htlc_object& htlc_obj = d.get_htlc(op.htlc_id);
		d.adjust_balance(htlc_obj.depositor, htlc_obj.amount);
		d.remove(op.htlc_id);
		return void_result();
	} FC_CAPTURE_AND_RETHROW((op)) }

} } // graphene::chain
