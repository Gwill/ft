// Copyright [2020] <Copyright Kevin, kevin.lau.gd@gmail.com>

#ifndef FT_SRC_TRADING_SYSTEM_TRADING_ENGINE_H_
#define FT_SRC_TRADING_SYSTEM_TRADING_ENGINE_H_

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "common/order.h"
#include "common/portfolio.h"
#include "core/account.h"
#include "core/config.h"
#include "core/error_code.h"
#include "core/gateway.h"
#include "core/trading_engine_interface.h"
#include "ipc/redis.h"
#include "risk_management/risk_manager.h"

namespace ft {

class TradingEngine : public TradingEngineInterface {
 public:
  TradingEngine();

  ~TradingEngine();

  bool login(const Config& config);

  void run();

  void close();

 private:
  bool send_order(const TraderCommand* cmd);

  void cancel_order(uint64_t order_id);

  void cancel_for_ticker(uint32_t ticker_index);

  void cancel_all();

  void on_query_contract(const Contract* contract) override;

  void on_query_account(const Account* account) override;

  void on_query_position(const Position* position) override;

  void on_query_trade(const Trade* trade) override;

  void on_tick(const TickData* tick) override;

  void on_order_accepted(uint64_t engine_order_id, uint64_t order_id) override;

  void on_order_rejected(uint64_t engine_order_id) override;

  void on_order_traded(uint64_t engine_order_id, uint64_t order_id,
                       int this_traded, double traded_price) override;

  void on_order_canceled(uint64_t engine_order_id,
                         int canceled_volume) override;

  void on_order_cancel_rejected(uint64_t engine_order_id) override;

 private:
  uint64_t next_engine_order_id() { return next_engine_order_id_++; }

  void respond_send_order_error(const TraderCommand* cmd,
                                int error_code = NO_ERROR);

 private:
  std::unique_ptr<Gateway> gateway_{nullptr};

  ProtocolQueryCenter proto_{};
  Account account_{};
  Portfolio portfolio_;
  std::unique_ptr<RiskManager> risk_mgr_{nullptr};
  std::map<uint64_t, Order> order_map_{};
  std::mutex mutex_{};

  uint64_t next_engine_order_id_{1};

  RedisSession tick_redis_{};
  RedisSession order_redis_{};

  std::atomic<bool> is_logon_{false};
};

}  // namespace ft

#endif  // FT_SRC_TRADING_SYSTEM_TRADING_ENGINE_H_
