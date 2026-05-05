/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 with LLVM Exceptions (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <catch2/catch_all.hpp>
#include <stdexec/execution.hpp>

#include <test_common/receivers.hpp>

#include <thread>
#include <type_traits>

namespace ex = STDEXEC;

namespace
{
  TEST_CASE("run_loop::scheduler::get_run_loop returns the parent loop",
            "[scheduler][run_loop]")
  {
    ex::run_loop loop;
    auto         sched = loop.get_scheduler();

    // The accessor returns a reference to the parent run_loop.
    auto& recovered = sched.get_run_loop();

    // It is the *same* loop, by address.
    CHECK(&recovered == &loop);
  }

  TEST_CASE("run_loop::scheduler::get_run_loop is noexcept and returns run_loop&",
            "[scheduler][run_loop]")
  {
    ex::run_loop loop;
    auto const   sched = loop.get_scheduler();

    STATIC_REQUIRE(noexcept(sched.get_run_loop()));
    STATIC_REQUIRE(::std::is_same_v<decltype(sched.get_run_loop()), ex::run_loop&>);
  }

  TEST_CASE(
    "run_loop::scheduler::get_run_loop enables driving the loop from a callback",
    "[scheduler][run_loop]")
  {
    // The motivating use case: a consumer is given a `run_loop::scheduler`
    // (e.g. via a sender pipeline) and needs to call `loop.finish()` from a
    // completion callback so that a separate thread driving `loop.run()`
    // can return. Before this accessor, recovering the `run_loop&` from the
    // scheduler required reading the private `__loop_` member of the
    // schedule sender's environment.

    ex::run_loop loop;
    auto         sched = loop.get_scheduler();

    bool ran = false;

    // Driver thread: blocks in run() until the work below calls finish().
    ::std::thread driver([&] { loop.run(); });

    auto work = ex::schedule(sched) | ex::then([&] {
                  ran = true;
                  // Recover the parent loop from the scheduler we were
                  // handed, with no access to internal members.
                  sched.get_run_loop().finish();
                });

    auto op = ex::connect(::std::move(work), expect_void_receiver{});
    ex::start(op);

    driver.join();

    CHECK(ran);
  }
} // namespace
