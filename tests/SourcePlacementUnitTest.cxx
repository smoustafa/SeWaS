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

#include <memory>

#include <gtest/gtest.h>

#include "CartesianMesh3D.hxx"
#include "CentralFDOperator.hxx"
#include "DataSet.hxx"
#include "LinearSeismicWaveModel.hxx"
#include "LogManager.hxx"
#include "Mesh3DPartitioning.hxx"
#include "MetricsManager.hxx"
#include "SEWASParameterManager.hxx"

namespace{
class SourcePlacementTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    LogManager::getInstance();
    MetricsManager::getInstance("SEWAS-test",
                                {"Global", "Core simulation", "Initialization", "ComputeVelocity", "ComputeStress"});
  }

  void TearDown() override
  {
    LinearSeismicWaveModel::releaseInstance();
    DataSet::releaseInstance();
    Mesh3DPartitioning::releaseInstance();
    CartesianMesh3D::releaseInstance();
    MetricsManager::releaseInstance();
    LogManager::releaseInstance();
  }

  LinearSeismicWaveModel * createModel(const char * dataFile)
  {
    int argc = 17;
    char arg0[] = "sewas";
    char arg1[] = "--cx";
    char arg2[] = "4";
    char arg3[] = "--cy";
    char arg4[] = "4";
    char arg5[] = "--cz";
    char arg6[] = "4";
    char arg7[] = "--P";
    char arg8[] = "1";
    char arg9[] = "--Q";
    char arg10[] = "1";
    char arg11[] = "--R";
    char arg12[] = "1";
    char arg13[] = "--nthreads";
    char arg14[] = "1";
    char arg15[] = "--dfile";
    char * arg16 = const_cast<char *>(dataFile);
    char* argvStorage[] = {arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,
                           arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16};
    char** argv = argvStorage;

    pm_ = std::make_unique<SEWASParameterManager>(&argc, &argv);

    const auto hnx = CentralFDOperator::hnx();
    const auto hny = CentralFDOperator::hny();
    const auto hnz = CentralFDOperator::hnz();

    EXPECT_NE(nullptr, CartesianMesh3D::getInstance(pm_->nx(), pm_->ny(), pm_->nz(), pm_->ds()));
    EXPECT_NE(nullptr, Mesh3DPartitioning::getInstance(pm_->cx(), pm_->cy(), pm_->cz(), hnx, hny, hnz,
                                                       pm_->P(), pm_->Q(), pm_->R()));

    CentralFDOperator fdo(CartesianMesh3D::getInstance()->dx(),
                          CartesianMesh3D::getInstance()->dy(),
                          CartesianMesh3D::getInstance()->dz());

    EXPECT_NE(nullptr, DataSet::getInstance(*pm_));
    return LinearSeismicWaveModel::getInstance(fdo, *pm_, pm_->nt(), pm_->tmax());
  }

  std::unique_ptr<SEWASParameterManager> pm_;
};
}

TEST_F(SourcePlacementTest, BoundaryCellStaysInFirstTile)
{
  auto * model = createModel("TestBoundarySource.json");
  ASSERT_NE(nullptr, model);

  const auto & leftTile = model->v(SWS::X)(0, 0, 0);
  const auto & rightTile = model->v(SWS::X)(1, 0, 0);

  ASSERT_TRUE(leftTile.hasSource());
  EXPECT_FALSE(rightTile.hasSource());
  ASSERT_EQ(1u, leftTile.is().size());
  EXPECT_EQ(leftTile.iEnd() - 1, leftTile.is().front());
}

TEST_F(SourcePlacementTest, MultipleTilesUseOnlyLocalSourceCoordinates)
{
  auto * model = createModel("TestMultipleSources.json");
  ASSERT_NE(nullptr, model);

  ASSERT_EQ(1u, model->v(SWS::X)(0, 0, 0).is().size());
  ASSERT_EQ(1u, model->v(SWS::X)(1, 0, 0).is().size());

  LinearSeismicWaveModel::initializeFieldsWrapper(0, 0, 0);
  LinearSeismicWaveModel::initializeFieldsWrapper(1, 0, 0);

  auto & leftTile = const_cast<SWS::SpatialBlockField<SWS::RealType>&>(model->v(SWS::X)(0, 0, 0));
  auto & rightTile = const_cast<SWS::SpatialBlockField<SWS::RealType>&>(model->v(SWS::X)(1, 0, 0));

  const auto leftIndex = leftTile.index(leftTile.is().front(), leftTile.js().front(), leftTile.ks().front());
  const auto rightIndex = rightTile.index(rightTile.is().front(), rightTile.js().front(), rightTile.ks().front());

  EXPECT_EQ(0.0, leftTile.data()(leftIndex));
  EXPECT_EQ(0.0, rightTile.data()(rightIndex));

  EXPECT_EQ(0, LinearSeismicWaveModel::computeVelocityWrapper(SWS::X, 2, 0, 0, 0));
  EXPECT_NE(0.0, leftTile.data()(leftIndex));
  EXPECT_EQ(0.0, rightTile.data()(rightIndex));

  EXPECT_EQ(0, LinearSeismicWaveModel::computeVelocityWrapper(SWS::X, 2, 1, 0, 0));
  EXPECT_NE(0.0, rightTile.data()(rightIndex));
}
