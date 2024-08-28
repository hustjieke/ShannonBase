//
// Copyright 2022 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/log/absl_check.h"

#define ABSL_TEST_CHECK ABSL_CHECK
#define ABSL_TEST_CHECK_OK ABSL_CHECK_OK
#define ABSL_TEST_CHECK_EQ ABSL_CHECK_EQ
#define ABSL_TEST_CHECK_NE ABSL_CHECK_NE
#define ABSL_TEST_CHECK_GE ABSL_CHECK_GE
#define ABSL_TEST_CHECK_LE ABSL_CHECK_LE
#define ABSL_TEST_CHECK_GT ABSL_CHECK_GT
#define ABSL_TEST_CHECK_LT ABSL_CHECK_LT
#define ABSL_TEST_CHECK_STREQ ABSL_CHECK_STREQ
#define ABSL_TEST_CHECK_STRNE ABSL_CHECK_STRNE
#define ABSL_TEST_CHECK_STRCASEEQ ABSL_CHECK_STRCASEEQ
#define ABSL_TEST_CHECK_STRCASENE ABSL_CHECK_STRCASENE

#define ABSL_TEST_DCHECK ABSL_DCHECK
#define ABSL_TEST_DCHECK_OK ABSL_DCHECK_OK
#define ABSL_TEST_DCHECK_EQ ABSL_DCHECK_EQ
#define ABSL_TEST_DCHECK_NE ABSL_DCHECK_NE
#define ABSL_TEST_DCHECK_GE ABSL_DCHECK_GE
#define ABSL_TEST_DCHECK_LE ABSL_DCHECK_LE
#define ABSL_TEST_DCHECK_GT ABSL_DCHECK_GT
#define ABSL_TEST_DCHECK_LT ABSL_DCHECK_LT
#define ABSL_TEST_DCHECK_STREQ ABSL_DCHECK_STREQ
#define ABSL_TEST_DCHECK_STRNE ABSL_DCHECK_STRNE
#define ABSL_TEST_DCHECK_STRCASEEQ ABSL_DCHECK_STRCASEEQ
#define ABSL_TEST_DCHECK_STRCASENE ABSL_DCHECK_STRCASENE

#define ABSL_TEST_QCHECK ABSL_QCHECK
#define ABSL_TEST_QCHECK_OK ABSL_QCHECK_OK
#define ABSL_TEST_QCHECK_EQ ABSL_QCHECK_EQ
#define ABSL_TEST_QCHECK_NE ABSL_QCHECK_NE
#define ABSL_TEST_QCHECK_GE ABSL_QCHECK_GE
#define ABSL_TEST_QCHECK_LE ABSL_QCHECK_LE
#define ABSL_TEST_QCHECK_GT ABSL_QCHECK_GT
#define ABSL_TEST_QCHECK_LT ABSL_QCHECK_LT
#define ABSL_TEST_QCHECK_STREQ ABSL_QCHECK_STREQ
#define ABSL_TEST_QCHECK_STRNE ABSL_QCHECK_STRNE
#define ABSL_TEST_QCHECK_STRCASEEQ ABSL_QCHECK_STRCASEEQ
#define ABSL_TEST_QCHECK_STRCASENE ABSL_QCHECK_STRCASENE

#include "gtest/gtest.h"
#include "absl/log/check_test_impl.inc"
