#include <gtest/gtest.h>
#include <orient/util/compresslib/intersection.h>
using namespace compressionLib;

TEST(compressLib, compress) {
    srand(123456789);
    constexpr size_t N = 100001;
    std::vector<uint32_t> orig(N), cmprs_out(N);
    for (uint32_t i = 0; i < N; ++i)
        orig[i] = 25 * i + rand() % 20;
    std::vector<uint32_t> orig_cpy(orig);

    fastPForCodec codec;
    size_t out_sz = cmprs_out.size();
    codec.encodeArray(orig_cpy.data(), orig_cpy.size(),
                      cmprs_out.data(), out_sz);
    cmprs_out.resize(out_sz); cmprs_out.shrink_to_fit();

    EXPECT_LT(out_sz * 5, orig.size()) << "Low (<5) compression rate!";
    EXPECT_NE(orig, orig_cpy);

    out_sz = orig_cpy.size();
    codec.decodeArray(cmprs_out.data(), cmprs_out.size(),
                      orig_cpy.data(), out_sz);
    EXPECT_EQ(out_sz, orig.size()) << "Bad decompressed size";
    EXPECT_EQ(out_sz, orig_cpy.size()) << "Bad decompressed size";
    EXPECT_EQ(orig_cpy, orig) << "Bad decompressed data";
}

TEST(compressLib, intersect) {
    std::vector<uint32_t> lhs(10000), rhs(10000), res(10, 12345678);
    for (uint32_t i = 0; i < 10000; ++i) {
        lhs[i] = i;
        rhs[i] = 20000 + i;
    }
    rhs.push_back(55556);

    EXPECT_EQ(i32_intersect(lhs.data(), lhs.size(), rhs.data(),
                            rhs.size(), res.data()), 0);
    EXPECT_EQ(i32_intersect(rhs.data(), rhs.size(), lhs.data(),
                            lhs.size(), res.data()), 0);

    lhs.push_back(25000); lhs.push_back(55000); lhs.push_back(65000);
    EXPECT_EQ(i32_intersect(lhs.data(), lhs.size(), rhs.data(),
                            rhs.size(), res.data()), 1);
    EXPECT_EQ(i32_intersect(rhs.data(), rhs.size(), lhs.data(),
                            lhs.size(), res.data()), 1);
    EXPECT_EQ(res[0], 25000);
    // Will write 16 bytes over end of result
    EXPECT_EQ(res[5], 12345678);
}
