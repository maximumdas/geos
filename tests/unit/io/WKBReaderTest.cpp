// $Id$
// 
// Test Suite for geos::io::WKBReader 
// Uses geos::io::WKTReader to check correctness 
// Uses geos::io::WKBWriter to check correctness 
// Currently only tests 2D geoms of all (7) types.
// Currently only tests NDR WKB 

// TUT
#include <tut.h>
// GEOS
#include <geos/io/WKBReader.h>
#include <geos/io/WKBConstants.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <sstream>
#include <memory>

namespace tut
{
	//
	// Test Group
	//

	// dummy data, not used
	struct test_wkbreader_data
	{
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKBReader wkbreader;
		geos::io::WKBWriter xdrwkbwriter;
		geos::io::WKBWriter ndrwkbwriter;
		geos::io::WKTReader wktreader;

		typedef std::auto_ptr<geos::geom::Geometry> GeomPtr;

		test_wkbreader_data()
			:
			pm(1.0),
			gf(&pm),
			wkbreader(gf),
			// 2D only, XDR (big endian)
			xdrwkbwriter(2, geos::io::WKBConstants::wkbXDR),
			// 2D only, NDR (little endian)
			ndrwkbwriter(2, geos::io::WKBConstants::wkbNDR),
			wktreader(&gf)
		{}

		void testInputOutput(const std::string& WKT,
				const std::string& ndrWKB,
				const std::string& xdrWKB)
		{
			GeomPtr gWKT(wktreader.read(WKT));

			// NDR input
			std::stringstream ndr_in(ndrWKB);
			GeomPtr gWKB_ndr(wkbreader.readHEX(ndr_in));
			ensure("NDR input",
				gWKB_ndr->equalsExact(gWKT.get()) );

			// XDR input
			std::stringstream xdr_in(xdrWKB);
			GeomPtr gWKB_xdr(wkbreader.readHEX(xdr_in));
			ensure("XDR input",
				gWKB_xdr->equalsExact(gWKT.get()) );

			// Compare geoms read from NDR and XDR
			ensure( gWKB_xdr->equalsExact(gWKB_ndr.get()) );

			// NDR output
			std::stringstream ndr_out;
			ndrwkbwriter.writeHEX(*gWKT, ndr_out);
			ensure_equals("NDR output",
				ndr_out.str(), ndr_in.str());

			// XDR output
			std::stringstream xdr_out;
			xdrwkbwriter.writeHEX(*gWKT, xdr_out);
			ensure_equals("XDR output",
				xdr_out.str(), xdr_in.str());

		}

	};

	typedef test_group<test_wkbreader_data> group;
	typedef group::object object;

	group test_wkbreader_group("geos::io::WKBReader");


	//
	// Test Cases
	//

	// 1 - Read/write a point in XDR and NDR format
	template<>
	template<>
	void object::test<1>()
	{         
		testInputOutput(

			// WKT
			"POINT(0 0)",

			// NDR HEXWKB
			"010100000000000000000000000000000000000000",

			// XDR HEXWKB
			"000000000100000000000000000000000000000000"

		);
	}

	// 2 - Read a linestring
	template<>
	template<>
	void object::test<2>()
	{         

		testInputOutput(

			// WKT
			"LINESTRING(1 2, 3 4)",

			// NDR HEXWKB
			"010200000002000000000000000000F03F000000000000004000000000000008400000000000001040",

			// XDR HEXWKB
			"0000000002000000023FF0000000000000400000000000000040080000000000004010000000000000"

		);

	}

	// 3 - Read a polygon
	template<>
	template<>
	void object::test<3>()
	{         
		testInputOutput(

			// WKT
			"POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2))",

			// NDR HEXWKB
			"0103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040",

			// XDR HEXWKB
			"0000000003000000020000000500000000000000000000000000000000402400000000000000000000000000004024000000000000402400000000000000000000000000004024000000000000000000000000000000000000000000000000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040000000000000004000000000000000"

		);

	}

	// 4 - Read a multipoint
	template<>
	template<>
	void object::test<4>()
	{         

		testInputOutput(

			// WKT
			"MULTIPOINT(0 0, 10 0, 10 10, 0 10, 0 0)",

			// NDR HEXWKB
			"010400000005000000010100000000000000000000000000000000000000010100000000000000000024400000000000000000010100000000000000000024400000000000002440010100000000000000000000000000000000002440010100000000000000000000000000000000000000",

			// XDR HEXWKB
			"000000000400000005000000000100000000000000000000000000000000000000000140240000000000000000000000000000000000000140240000000000004024000000000000000000000100000000000000004024000000000000000000000100000000000000000000000000000000"

		);

	}

	// 5 - Read a multilinestring
	template<>
	template<>
	void object::test<5>()
	{         

		testInputOutput(

			// WKT
			"MULTILINESTRING((0 0, 10 0, 10 10, 0 10, 10 20),(2 2, 2 6, 6 4, 20 2))",

			// NDR HEXWKB
			"010500000002000000010200000005000000000000000000000000000000000000000000000000002440000000000000000000000000000024400000000000002440000000000000000000000000000024400000000000002440000000000000344001020000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000034400000000000000040",

			// XDR HEXWKB
			"000000000500000002000000000200000005000000000000000000000000000000004024000000000000000000000000000040240000000000004024000000000000000000000000000040240000000000004024000000000000403400000000000000000000020000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040340000000000004000000000000000"

		);

	}

	// 6 - Read a multipolygon
	template<>
	template<>
	void object::test<6>()
	{         

		testInputOutput(

			// WKT
			"MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2)),((60 60, 60 50, 70 40, 60 60)))",

			// NDR HEXWKB
			"0106000000020000000103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040010300000001000000040000000000000000004E400000000000004E400000000000004E400000000000004940000000000080514000000000000044400000000000004E400000000000004E40",

			// XDR HEXWKB
			"000000000600000002000000000300000002000000050000000000000000000000000000000040240000000000000000000000000000402400000000000040240000000000000000000000000000402400000000000000000000000000000000000000000000000000044000000000000000400000000000000040000000000000004018000000000000401800000000000040100000000000004000000000000000400000000000000000000000030000000100000004404E000000000000404E000000000000404E000000000000404900000000000040518000000000004044000000000000404E000000000000404E000000000000"

		);

	}

	// 7 - Read a collection 
	template<>
	template<>
	void object::test<7>()
	{         

		testInputOutput(

			// WKT
			"GEOMETRYCOLLECTION(POINT(0 0),LINESTRING(1 2,3 4),POLYGON((0 0,10 0,10 10,0 10,0 0),(2 2,2 6,6 4,2 2)),MULTIPOINT(0 0,10 0,10 10,0 10,0 0),MULTILINESTRING((0 0,10 0,10 10,0 10,10 20),(2 2,2 6,6 4,20 2)),MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(2 2,2 6,6 4,2 2)),((60 60,60 50,70 40,60 60))))",

			// NDR HEXWKB
			"010700000006000000010100000000000000000000000000000000000000010200000002000000000000000000F03F00000000000000400000000000000840000000000000104001030000000200000005000000000000000000000000000000000000000000000000002440000000000000000000000000000024400000000000002440000000000000000000000000000024400000000000000000000000000000000004000000000000000000004000000000000000400000000000000040000000000000184000000000000018400000000000001040000000000000004000000000000000400104000000050000000101000000000000000000000000000000000000000101000000000000000000244000000000000000000101000000000000000000244000000000000024400101000000000000000000000000000000000024400101000000000000000000000000000000000000000105000000020000000102000000050000000000000000000000000000000000000000000000000024400000000000000000000000000000244000000000000024400000000000000000000000000000244000000000000024400000000000003440010200000004000000000000000000004000000000000000400000000000000040000000000000184000000000000018400000000000001040000000000000344000000000000000400106000000020000000103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040010300000001000000040000000000000000004E400000000000004E400000000000004E400000000000004940000000000080514000000000000044400000000000004E400000000000004E40",

			// XDR HEXWKB
			"0000000007000000060000000001000000000000000000000000000000000000000002000000023FF00000000000004000000000000000400800000000000040100000000000000000000003000000020000000500000000000000000000000000000000402400000000000000000000000000004024000000000000402400000000000000000000000000004024000000000000000000000000000000000000000000000000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040000000000000004000000000000000000000000400000005000000000100000000000000000000000000000000000000000140240000000000000000000000000000000000000140240000000000004024000000000000000000000100000000000000004024000000000000000000000100000000000000000000000000000000000000000500000002000000000200000005000000000000000000000000000000004024000000000000000000000000000040240000000000004024000000000000000000000000000040240000000000004024000000000000403400000000000000000000020000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040340000000000004000000000000000000000000600000002000000000300000002000000050000000000000000000000000000000040240000000000000000000000000000402400000000000040240000000000000000000000000000402400000000000000000000000000000000000000000000000000044000000000000000400000000000000040000000000000004018000000000000401800000000000040100000000000004000000000000000400000000000000000000000030000000100000004404E000000000000404E000000000000404E000000000000404900000000000040518000000000004044000000000000404E000000000000404E000000000000"

		);

	}


} // namespace tut

