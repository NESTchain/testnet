#pragma once

#include "name.hpp"
#include "action.hpp"
#include "transaction_context.hpp"

using account_name = eosio::chain::name;

namespace eosio { namespace chain {

	class  apply_context {
		public:
            apply_context(transaction_context& t, const action& a, account_name r)
                :trx_context(t), act(a), receiver(r)
            {}

			transaction_context&          trx_context; ///< transaction context in which the action is running
			const action&                 act; ///< message being applied
			account_name                  receiver; ///< the code that is currently running

	};
}
}
