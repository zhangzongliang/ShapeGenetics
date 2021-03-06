/*
 * ThreeBranchTreeGrammar_v2.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: Karl Haubenwallner
 */

#include <iostream>
#include "operators/Generator.impl"
#include "operators/ScopeOperators.impl"
#include "operators/Resize.impl"
#include "operators/Repeat.impl"
#include "operators/Subdivide.impl"
#include "operators/ComponentSplit.impl"
#include "operators/Extrude.impl"
#include "operators/RandomPath.impl"
#include "operators/Duplicate.impl"
#include "parameters/StaticParameter.h"
#include "parameters/StaticRandom.h"
#include "parameters/ParameterConversion.h"
#include "parameters/Random.h"
#include "parameters/ShapeParameter.h"
#include "parameters/ScopeParameter.h"
#include "parameters/ParameterOperations.h"
#include "modifiers/DirectCall.h"
#include "modifiers/RandomReseed.h"
#include "modifiers/Discard.h"
#include "modifiers/ScopeModifier.h"
#include "CPU/StaticCall.h"
#include "GeometryGeneratorInstanced.h"
#include "ThreeBranchTreeGrammar_v2.h"


using namespace PGG;
using namespace Shapes;
using namespace Parameters;
using namespace Scope;
using namespace Operators;
using namespace Modifiers;
using namespace CPU;

namespace PGA {

namespace Tree {

void ThreeBranchTreeGrammar_v2::initSymbols(SymbolManager& sm)
{
	// start symbol, has no shape
	S = sm.createStart("S");

	// Body element
	Trunk = sm.createTerminal("T");
	sm.addParameter<math::float3>(Trunk, "size", math::float3(0.8f, 1.4f, 0.8f), math::float3(0.8f, 1.8f, 0.8f));

	Branch1 = sm.createTerminal("b");
	sm.addParameter<math::float3>(Branch1, "branch_size", math::float3(0.6, 0.6, 0.6f), math::float3(0.7, 0.801f, 0.7f));
	sm.addParameter<float>(Branch1, "rotate_y", 0.0f, 120.0f);
	sm.addParameter<float>(Branch1, "branch_rotate_z", 35.0f, 85.0f);
	sm.addParameter<float>(Branch1, "branch_rotate_y", -3.0f, 3.0f);

	Branch2 = sm.createTerminal("n");
	sm.addParameter<math::float3>(Branch2, "branch_size", math::float3(0.6, 0.6, 0.6f), math::float3(0.7, 0.801f, 0.7f));
	sm.addParameter<float>(Branch2, "rotate_y", 120.0f, 240.0f);
	sm.addParameter<float>(Branch2, "branch_rotate_z", 35.0f, 85.0f);
	sm.addParameter<float>(Branch2, "branch_rotate_y", -3.0f, 3.0f);


	Branch3 = sm.createTerminal("m");
	sm.addParameter<math::float3>(Branch3, "branch_size", math::float3(0.6, 0.6, 0.6f), math::float3(0.7, 0.801f, 0.7f));
	sm.addParameter<float>(Branch3, "rotate_y", 240.0f, 360.0f);
	sm.addParameter<float>(Branch3, "branch_rotate_z", 35.0f, 85.0f);
	sm.addParameter<float>(Branch3, "branch_rotate_y", -3.0f, 3.0f);


	sm.addPossibleChild(S, Trunk, 1, 1.0f);

	sm.addPossibleChild(Trunk, Branch1, 1, 1.0f/3.0f);
	sm.addPossibleChild(Trunk, Branch2, 1, 1.0f/3.0f);
	sm.addPossibleChild(Trunk, Branch3, 1, 1.0f/3.0f);

	sm.addPossibleChild(Branch1, Branch1, 1, 1.0f/3.0f);
	sm.addPossibleChild(Branch1, Branch2, 1, 1.0f/3.0f);
	sm.addPossibleChild(Branch1, Branch3, 1, 1.0f/3.0f);

	sm.addPossibleChild(Branch2, Branch1, 1, 1.0f/3.0f);
	sm.addPossibleChild(Branch2, Branch2, 1, 1.0f/3.0f);
	sm.addPossibleChild(Branch2, Branch3, 1, 1.0f/3.0f);

	sm.addPossibleChild(Branch3, Branch1, 1, 1.0f/3.0f);
	sm.addPossibleChild(Branch3, Branch2, 1, 1.0f/3.0f);
	sm.addPossibleChild(Branch3, Branch3, 1, 1.0f/3.0f);

	if (getNumPreparedShapes() != 2) {
		throw std::invalid_argument("Wrong number of shapes for the Grammar");
	}

}

int ThreeBranchTreeGrammar_v2::storeParameter(Symbol* symbol, PGG::Parameters::ParameterTable& pt)
{

	if (symbol->id() == Branch1 || symbol->id() == Branch2 || symbol->id() == Branch3) {
		return storeBranchParameter(symbol, pt);
	}

	if (symbol->id() == Trunk) {
		return storeTrunkParameter(symbol, pt);
	}

	if (symbol->id() == S)
		return -1;

	//std::cout << "no Parameters for Symbol id " << symbol->id() << std::endl;
	return -1;

}

int ThreeBranchTreeGrammar_v2::storeTrunkParameter(Symbol* s, PGG::Parameters::ParameterTable& pt)
{
	int branch[3] = {0};
	int branch_offset[3] = {0};

	for (int i = 0; i < s->getChildren().size(); ++i) {
		Symbol* c = s->getChildren().at(i);
		if (c->id() == Branch1) {
			branch[0] = 1;
			branch_offset[0] = c->getParamTableOffset();
		}
		if (c->id() == Branch2) {
			branch[1] = 1;
			branch_offset[1] = c->getParamTableOffset();
		}
		if (c->id() == Branch3) {
			branch[2] = 1;
			branch_offset[2] = c->getParamTableOffset();
		}
	}

	math::float3 size = s->getParameter()[0]->getValue<math::float3>();

	int pt_offset = pt.storeParameters(	size,
										branch[0],			 	// attach the first branch
										branch_offset[0],		// Param Layer of first branch
										branch[1],			 	// attach the second branch
										branch_offset[1],		// Param Layer of second branch
										branch[2],			 	// attach the third branch
										branch_offset[2]);		// Param Layer of third branch

	return pt_offset;

}

int ThreeBranchTreeGrammar_v2::storeBranchParameter(Symbol* s, PGG::Parameters::ParameterTable& pt)
{
	int branch[3] = {0};
	int branch_offset[3] = {0};
	for (int i = 0; i < s->getChildren().size(); ++i) {
		Symbol* c = s->getChildren().at(i);
		if (c->id() == Branch1) {
			branch[0] = 1;
			branch_offset[0] = c->getParamTableOffset();
		}
		if (c->id() == Branch2) {
			branch[1] = 1;
			branch_offset[1] = c->getParamTableOffset();
		}
		if (c->id() == Branch3) {
			branch[2] = 1;
			branch_offset[2] = c->getParamTableOffset();
		}
	}

//	math::float3 size = s->getParameter()[0]->getValue<math::float3>();


	math::float3 branch_size = s->getParameter()[0]->getValue<math::float3>();
	float y_rotation = s->getParameter()[1]->getValue<float>();
	float branch_rotation_z = s->getParameter()[2]->getValue<float>();
	float branch_rotation_y = s->getParameter()[3]->getValue<float>();

	int pt_offset = pt.storeParameters(	(y_rotation * math::constants<float>::pi()) / 180.0f,
										(branch_rotation_z  * math::constants<float>::pi()) / 180.0f,
										branch_size,
										(branch_rotation_y * math::constants<float>::pi()) / 180.0f,
										branch[0],			 	// attach the first branch
										branch_offset[0],		// Param Layer of first branch
										branch[1],			 	// attach the second branch
										branch_offset[1],		// Param Layer of second branch
										branch[2],			 	// attach the third branch
										branch_offset[2]);		// Param Layer of third branch

	return pt_offset;

}

void ThreeBranchTreeGrammar_v2::createAxiom(PGG::CPU::GrammarSystem& system, const int axiomId)
{

	//forward declaratons for recursion
	class Tree;
	class GenerateBranch;
	class FirstBranchRule;
	class SecondBranchRule;
	class GenerateBranchRule;
	class GenerateLeaf1Rule;
	class GenerateLeaf2Rule;
	class GenerateLeaf3Rule;
	class BranchRecursionFromTrunk;
	class BranchRecursion;


	typedef CoordinateframeScope<int> TreeScope;
	typedef PGA::InstancedShapeGenerator<TreeScope, 0, true> TreeGenerator;
	typedef PGA::InstancedShapeGenerator<TreeScope, 1, false> LeafGenerator;

	typedef Operators::Generator<TreeGenerator> GenerateTree;
	typedef Operators::Generator<LeafGenerator> GenerateLeaf;



	static const int ParamLayer = 0;

	// offset 0: float3 size of trunk
	// offset 3: branch recursion offset
	class Tree : public
		Resize<DynamicFloat3<0, ParamLayer>,
			Duplicate<
				DirectCall<GenerateTree>,
				DirectCall<Translate< Vec< StaticFloat<0.0_p>, Mul < StaticFloat<0.0_p>, ShapeSizeAxis<Axes::YAxis> >, StaticFloat<0.0_p> >,
				StaticCall<BranchRecursionFromTrunk>
			  >
			>
			>
		>
	{ };

	// offset 3: has first branch
	// offset 4: first branch offset
	// offset 5: has second branch
	// offset 6: second branch offset
	// offset 7: has third branch
	// offset 8: third branch offset
	class BranchRecursionFromTrunk : public
		DirectCall< Duplicate< DirectCall< ChoosePath< DynamicInt< 3, ParamLayer>,
											 SetScopeAttachment<ParamLayer, DynamicInt<4, ParamLayer>, StaticCall<GenerateBranchRule> > > >,
		                       DirectCall< ChoosePath< DynamicInt< 5, ParamLayer>,
											 SetScopeAttachment<ParamLayer, DynamicInt<6, ParamLayer>, StaticCall<GenerateBranchRule>	> > >,
		                       DirectCall< ChoosePath< DynamicInt< 7, ParamLayer>,
											 SetScopeAttachment<ParamLayer, DynamicInt<8, ParamLayer>, StaticCall<GenerateBranchRule> > > >
		>  >
	{};


	// offset 8: has first branch
	// offset 9: first branch offset
	// offset 10: has second branch
	// offset 11: second branch offset
	// offset 12: has third branch
	// offset 13: third branch offset
	class BranchRecursion : public
	  DirectCall< ChoosePath< LogicalNot<LogicalOr< DynamicInt<12, ParamLayer>, LogicalOr< DynamicInt< 8, ParamLayer>, DynamicInt< 10, ParamLayer> > > >,
	  	  	  StaticCall<GenerateLeaf1Rule>,
	  	  	  StaticInt<1>,
		DirectCall< Duplicate< DirectCall< ChoosePath< DynamicInt< 8, ParamLayer>,
											 SetScopeAttachment<ParamLayer, DynamicInt<9, ParamLayer>, StaticCall<GenerateBranchRule> > > >,
		                       DirectCall< ChoosePath< DynamicInt< 10, ParamLayer>,
											 SetScopeAttachment<ParamLayer, DynamicInt<11, ParamLayer>, StaticCall<GenerateBranchRule> > > >,
											// StaticInt<1>, StaticCall<GenerateLeaf2Rule> > >,
		                       DirectCall< ChoosePath< DynamicInt< 12, ParamLayer>,
											 SetScopeAttachment<ParamLayer, DynamicInt<13, ParamLayer>, StaticCall<GenerateBranchRule> > > > //,
											// StaticInt<1>, StaticCall<GenerateLeaf3Rule> > >
		>  >
	> >
	{};

	// offset 0: rotate along Y-Axis
	// offset 1: rotate along Z-Axis
	// offset 2, 3: padding
	// offset 4: resize
	// offset 7: rotate along y-axis
	class GenerateBranchRule : public
	// --------------------------------------- first branch:  ------------------
	Rotate<StaticAxes<Axes::YAxis>, DynamicFloat<0, ParamLayer>,
	DirectCall<Translate< Vec< StaticFloat<0.0_p>, Mul< StaticFloat<0.38_p>, ShapeSizeAxis<Axes::YAxis> >, StaticFloat<0.0_p> >,
	      DirectCall<Rotate<StaticAxes<Axes::ZAxis>, DynamicFloat<1, ParamLayer>,
	            DirectCall<Translate<Vec< StaticFloat<0.0_p>, Mul< StaticFloat<0.0_p>, ShapeSizeAxis<Axes::YAxis> >, StaticFloat<0.0_p> >,
	            	DirectCall<Resize<DynamicFloat3<4, ParamLayer>,
	            //DirectCall<Resize<Vec< StaticFloat<0.7_p>, StaticFloat<0.8_p>, StaticFloat<0.7_p> >,
	            		DirectCall< Translate< Vec< StaticFloat<0.0_p>, Mul< StaticFloat<0.5_p>, ShapeSizeAxis<Axes::YAxis> >, StaticFloat<0.0_p> >,
	            			DirectCall<Rotate<StaticAxes<Axes::YAxis>, DynamicFloat<7, ParamLayer>,
	            				Duplicate< StaticCall<GenerateTree>, StaticCall<BranchRecursion> >
							> >
						> >
					> >
				> >
			> >
	> >
	>
	{};

	class GenerateLeaf1Rule : public
	// --------------------------------------- first branch:  ------------------
	Translate< Vec< StaticFloat<0.0_p>, Mul< StaticFloat<0.75_p>, ShapeSizeAxis<Axes::YAxis> >, StaticFloat<0.0_p> >,
     	 DirectCall<GenerateLeaf> >
	{};

	class GenerateLeaf2Rule : public
	// --------------------------------------- first branch:  ------------------
	Translate< Vec< StaticFloat<0.0_p>, Mul< StaticFloat<0.5_p>, ShapeSizeAxis<Axes::YAxis> >, StaticFloat<0.0_p> >,
		DirectCall<Rotate<StaticAxes<Axes::ZAxis>, StaticFloat<2.5_p>,
		 DirectCall<Rotate<StaticAxes<Axes::YAxis>, StaticFloat<5.0_p>,
     	 DirectCall<GenerateLeaf> > > > > >
	{};

	class GenerateLeaf3Rule : public
	// --------------------------------------- first branch:  ------------------
	Translate< Vec< StaticFloat<0.0_p>, Mul< StaticFloat<0.9_p>, ShapeSizeAxis<Axes::YAxis> >, StaticFloat<0.0_p> >,
	DirectCall<Rotate<StaticAxes<Axes::ZAxis>, StaticFloat<4.0_p>,
		 DirectCall<Rotate<StaticAxes<Axes::YAxis>, StaticFloat<5.0_p>,
     	 DirectCall<GenerateLeaf> > > > > >
	{};

	ScopedShape<Box, TreeScope > treeAxiom(Box(math::float3(1.0f)), TreeScope(math::identity<math::float3x4>(), axiomId));

	system.addAxiom<Tree>(treeAxiom);

}


}; // namespace Tree

}; // namespace PGA
