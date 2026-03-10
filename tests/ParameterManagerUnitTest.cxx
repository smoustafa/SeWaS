/*
  SeWaS
  Copyright (C) 2018  ANEO

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
==============================================================================*/

#include <gtest/gtest.h>

#include "SEWASParameterManager.hxx"

TEST(SEWASParameterManagerTest, HelpExitsSuccessfully)
{
  int argc = 2;
  char arg0[] = "sewas";
  char arg1[] = "--help";
  char* argvStorage[] = {arg0, arg1};
  char** argv = argvStorage;

  EXPECT_EXIT(
    {
      SEWASParameterManager pm(&argc, &argv);
      (void)pm;
    },
    ::testing::ExitedWithCode(0),
    "Allowed options");
}
