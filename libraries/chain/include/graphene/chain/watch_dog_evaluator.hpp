/*
* Copyright (c) 2015 Cryptonomex, Inc., and contributors.
* Copyright (c) 2018- ��NEST Foundation, and contributors.
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

namespace graphene { namespace chain {

	class create_watch_dog_evaluator : public evaluator
	{
	public:
		typedef create_watch_dog_operation operation_type;

		void_result do_evaluate(const operation_type& op);
		object_id_type do_apply(const operation_type& op);

		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class prepare_watch_dog_evaluator : public evaluator
	{
	public:
		typedef prepare_watch_dog_operation operation_type;

		void_result do_evaluate(const operation_type& op);
		void_result do_apply(const operation_type& op);

		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class remind_account_evaluator : public evaluator
	{
	public:
		typedef remind_account_operation operation_type;

		void_result do_evaluate(const operation_type& op);
		void_result do_apply(const operation_type& op);

		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class answer_watch_dog_evaluator : public evaluator
	{
	public:
		typedef answer_watch_dog_operation operation_type;

		void_result do_evaluate(const operation_type& op);
		void_result do_apply(const operation_type& op);

		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class start_account_recover_evaluator : public evaluator
	{
	public:
		typedef start_account_recover_operation operation_type;

		void_result do_evaluate(const operation_type& op);
		void_result do_apply(const operation_type& op);

		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class transfer_lost_asset_evaluator : public evaluator
	{
	public:
		typedef transfer_lost_asset_operation operation_type;

		void_result do_evaluate(const operation_type& op);
		void_result do_apply(const operation_type& op);

		EVALUATOR_VIRTUAL_FUNCTIONS
	};

	class delete_watch_dog_evaluator : public evaluator
	{
	public:
		typedef delete_watch_dog_operation operation_type;

		void_result do_evaluate(const operation_type& op);
		void_result do_apply(const operation_type& op);

		EVALUATOR_VIRTUAL_FUNCTIONS
	};

} }
