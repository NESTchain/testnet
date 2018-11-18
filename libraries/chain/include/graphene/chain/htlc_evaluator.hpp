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

#pragma once
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

	class htlc_prepare_evaluator : public evaluator
	{
	public:
		typedef htlc_prepare_operation operation_type;

		void_result do_evaluate(const htlc_prepare_operation& op);
		object_id_type do_apply(const htlc_prepare_operation& op);
		
		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class htlc_redeem_evaluator : public evaluator
	{
	public:
		typedef htlc_redeem_operation operation_type;

		void_result do_evaluate(const htlc_redeem_operation& op);
		void_result do_apply(const htlc_redeem_operation& op);
		
		EVALUATOR_VIRTUAL_FUNCTIONS

	private:
		template<typename T>  // T is hash algorithm class
		void check_preimage_hash(const htlc_redeem_operation& op, const htlc_object& obj)
		{
			FC_ASSERT(T::hash(op.preimage) == T(obj.preimage_hash), "invalid preimage submitted");
		}
	};

	class htlc_extend_expiry_evaluator : public evaluator
	{
	public:
		typedef htlc_extend_expiry_operation operation_type;

		void_result do_evaluate(const htlc_extend_expiry_operation& op);
		void_result do_apply(const htlc_extend_expiry_operation& op);
		
		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class htlc_refund_evaluator : public evaluator
	{
	public:
		typedef htlc_refund_operation operation_type;

		void_result do_evaluate(const htlc_refund_operation& op);
		void_result do_apply(const htlc_refund_operation& op);
		
		EVALUATOR_VIRTUAL_FUNCTIONS
	};

} } // graphene::chain
