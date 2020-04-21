// Copyright [2020] <Copyright Kevin, kevin.lau.gd@gmail.com>

#include <iostream>

#include <getopt.hpp>
#include <spdlog/spdlog.h>

#include "Strategy.h"
#include "TradingSystem.h"

const char* kSimnowTradeAddr[] = {
  "tcp://180.168.146.187:10130",
  "tcp://180.168.146.187:10100",
  "tcp://180.168.146.187:10101",
  "tcp://218.202.237.33:10102"
};

const char* kSimnowMdAddr[] = {
  "tcp://180.168.146.187:10131",
  "tcp://180.168.146.187:10110",
  "tcp://180.168.146.187:10111",
  "tcp://218.202.237.33:10112"
};

const char* kBrokerID = "9999";
const char* kInvestorID = "122899";
const char* kPasswd = "lsk4129691";
const char* kAuthCode = "0000000000000000";
const char* kAppID = "simnow_client_test";

class MyStrategy : public ft::Strategy {
 public:
  void on_init(ft::QuantitativeTradingContext* ctx) override {
    spdlog::info("[MyStrategy::on_init]");

    const auto pos = ctx->get_position();
    const auto& lp = pos.long_pos;
    const auto& sp = pos.short_pos;

    if (lp.volume > 0) {
      ctx->sell(lp.volume, 3300);
      spdlog::info("Close all long pos");
    }

    if (sp.volume > 0) {
      ctx->buy(sp.volume, 3600);
      spdlog::info("Close all short pos");
    }
  }

  void on_tick(ft::QuantitativeTradingContext* ctx) override {
    auto* tick = ctx->get_tick();

    if (price_ <= 1e-6)
      price_ = tick->last_price;

    const auto pos = ctx->get_position();
    const auto& lp = pos.long_pos;
    const auto& sp = pos.short_pos;

    spdlog::info(
      "[MyStrategy::on_tick] last_price: {:.2f}, grid: {:.2f}, long: {}, short: {}, trades: {}",
      ctx->get_tick()->last_price, price_, lp.volume, sp.volume, trades_);

    if (tick->last_price - price_ >= grid_ - 1e-6) {
      ctx->sell(volume_, tick->bid[0]);
      spdlog::info("[GRID] SELL VOLUME: {}, PRICE: {:.2f}, LAST:{:.2f}, PREV: {:.2f}",
                   volume_, tick->bid[0], tick->last_price, price_);
      price_ = tick->last_price;
      ++trades_;
    } else if (tick->last_price - price_ <= -grid_ + 1e-6) {
      ctx->buy(volume_, tick->ask[0]);
      spdlog::info("[GRID] BUY VOLUME: {}, PRICE: {:.2f}, LAST:{:.2f}, PREV: {:.2f}",
                   volume_, tick->ask[0], tick->last_price, price_);
      price_ = tick->last_price;
      ++trades_;
    }
  }

  void on_exit(ft::QuantitativeTradingContext* ctx) override {
    spdlog::info("[MyStrategy::on_exit]");
  }

 private:
  double price_ = 0.0;
  double grid_ = 10.0;
  int volume_ = 100;
  int trades_ = 0;
};

int main() {
  std::size_t front_index = getarg(0UL, "--front");
  int log_level = getarg(2, "--loglevel");

  spdlog::set_level(static_cast<spdlog::level::level_enum>(log_level));

  ft::ContractTable::init("./contracts.csv");
  ft::TradingSystem ts(ft::FrontType::CTP);
  ft::LoginParams params;
  const std::string ticker = "rb2009.SHFE";

  if (front_index >= sizeof(kSimnowTradeAddr) / sizeof(kSimnowTradeAddr[0]))
    exit(-1);

  params.set_front_addr(kSimnowTradeAddr[front_index]);
  params.set_md_server_addr(kSimnowMdAddr[front_index]);
  params.set_broker_id(kBrokerID);
  params.set_investor_id(kInvestorID);
  params.set_passwd(kPasswd);
  params.set_auth_code(kAuthCode);
  params.set_app_id(kAppID);
  params.set_subscribed_list({ticker});

  if (!ts.login(params))
    return -1;

  MyStrategy strategy;
  ts.mount_strategy("rb2009.SHFE", &strategy);

  while (1) {
    sleep(60);
    // ts.show_positions();
  }
}
