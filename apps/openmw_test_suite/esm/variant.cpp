#include <components/esm3/variant.hpp>
#include <components/esm/esmwriter.hpp>
#include <components/esm3/reader.hpp>
#include <components/esm3/glob.hpp>
#include <components/esm3/gmst.hpp>
#include <components/esm3/info.hpp>
#include <components/esm3/globalscript.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <functional>

namespace
{
    using namespace testing;
    using namespace ESM3;

    Variant makeVariant(ESM::VarType type)
    {
        Variant v;
        v.setType(type);
        return v;
    }

    Variant makeVariant(ESM::VarType type, int value)
    {
        Variant v;
        v.setType(type);
        v.setInteger(value);
        return v;
    }

    TEST(ESMVariantTest, move_constructed_should_have_data)
    {
        Variant a(int{42});
        const Variant b(std::move(a));
        ASSERT_EQ(b.getInteger(), 42);
    }

    TEST(ESMVariantTest, copy_constructed_is_equal_to_source)
    {
        const Variant a(int{42});
        const Variant b(a);
        ASSERT_EQ(a, b);
    }

    TEST(ESMVariantTest, copy_constructed_does_not_share_data_with_source)
    {
        const Variant a(int{42});
        Variant b(a);
        b.setInteger(13);
        ASSERT_EQ(a.getInteger(), 42);
        ASSERT_EQ(b.getInteger(), 13);
    }

    TEST(ESMVariantTest, move_assigned_should_have_data)
    {
        Variant b;
        {
            Variant a(int{42});
            b = std::move(a);
        }
        ASSERT_EQ(b.getInteger(), 42);
    }

    TEST(ESMVariantTest, copy_assigned_is_equal_to_source)
    {
        const Variant a(int{42});
        Variant b;
        b = a;
        ASSERT_EQ(a, b);
    }

    TEST(ESMVariantTest, not_equal_is_negation_of_equal)
    {
        const Variant a(int{42});
        Variant b;
        b = a;
        ASSERT_TRUE(!(a != b));
    }

    TEST(ESMVariantTest, different_types_are_not_equal)
    {
        ASSERT_NE(Variant(int{42}), Variant(float{2.7f}));
    }

    struct ESMVariantWriteToOStreamTest : TestWithParam<std::tuple<Variant, std::string>> {};

    TEST_P(ESMVariantWriteToOStreamTest, should_write)
    {
        const auto [variant, result] = GetParam();
        std::ostringstream s;
        s << variant;
        ASSERT_EQ(s.str(), result);
    }

    INSTANTIATE_TEST_SUITE_P(VariantAsString, ESMVariantWriteToOStreamTest, Values(
        std::make_tuple(Variant(), "variant none"),
        std::make_tuple(Variant(int{42}), "variant long: 42"),
        std::make_tuple(Variant(float{2.7f}), "variant float: 2.7"),
        std::make_tuple(Variant(std::string("foo")), "variant string: \"foo\""),
        std::make_tuple(makeVariant(ESM::VT_Unknown), "variant unknown"),
        std::make_tuple(makeVariant(ESM::VT_Short, 42), "variant short: 42"),
        std::make_tuple(makeVariant(ESM::VT_Int, 42), "variant int: 42")
    ));

    struct ESMVariantGetTypeTest : Test {};

    TEST(ESMVariantGetTypeTest, default_constructed_should_return_none)
    {
        ASSERT_EQ(Variant().getType(), ESM::VT_None);
    }

    TEST(ESMVariantGetTypeTest, for_constructed_from_int_should_return_long)
    {
        ASSERT_EQ(Variant(int{}).getType(), ESM::VT_Long);
    }

    TEST(ESMVariantGetTypeTest, for_constructed_from_float_should_return_float)
    {
        ASSERT_EQ(Variant(float{}).getType(), ESM::VT_Float);
    }

    TEST(ESMVariantGetTypeTest, for_constructed_from_lvalue_string_should_return_string)
    {
        const std::string string;
        ASSERT_EQ(Variant(string).getType(), ESM::VT_String);
    }

    TEST(ESMVariantGetTypeTest, for_constructed_from_rvalue_string_should_return_string)
    {
        ASSERT_EQ(Variant(std::string{}).getType(), ESM::VT_String);
    }

    struct ESMVariantGetIntegerTest : Test {};

    TEST(ESMVariantGetIntegerTest, for_default_constructed_should_throw_exception)
    {
        ASSERT_THROW(Variant().getInteger(), std::runtime_error);
    }

    TEST(ESMVariantGetIntegerTest, for_constructed_from_int_should_return_same_value)
    {
        const Variant variant(int{42});
        ASSERT_EQ(variant.getInteger(), 42);
    }

    TEST(ESMVariantGetIntegerTest, for_constructed_from_float_should_return_casted_to_int)
    {
        const Variant variant(float{2.7});
        ASSERT_EQ(variant.getInteger(), 2);
    }

    TEST(ESMVariantGetIntegerTest, for_constructed_from_string_should_throw_exception)
    {
        const Variant variant(std::string("foo"));
        ASSERT_THROW(variant.getInteger(), std::runtime_error);
    }

    TEST(ESMVariantGetFloatTest, for_default_constructed_should_throw_exception)
    {
        ASSERT_THROW(Variant().getFloat(), std::runtime_error);
    }

    TEST(ESMVariantGetFloatTest, for_constructed_from_int_should_return_casted_to_float)
    {
        const Variant variant(int{42});
        ASSERT_EQ(variant.getFloat(), 42);
    }

    TEST(ESMVariantGetFloatTest, for_constructed_from_float_should_return_same_value)
    {
        const Variant variant(float{2.7f});
        ASSERT_EQ(variant.getFloat(), 2.7f);
    }

    TEST(ESMVariantGetFloatTest, for_constructed_from_string_should_throw_exception)
    {
        const Variant variant(std::string("foo"));
        ASSERT_THROW(variant.getFloat(), std::runtime_error);
    }

    TEST(ESMVariantGetStringTest, for_default_constructed_should_throw_exception)
    {
        ASSERT_THROW(Variant().getString(), std::bad_variant_access);
    }

    TEST(ESMVariantGetStringTest, for_constructed_from_int_should_throw_exception)
    {
        const Variant variant(int{42});
        ASSERT_THROW(variant.getString(), std::bad_variant_access);
    }

    TEST(ESMVariantGetStringTest, for_constructed_from_float_should_throw_exception)
    {
        const Variant variant(float{2.7});
        ASSERT_THROW(variant.getString(), std::bad_variant_access);
    }

    TEST(ESMVariantGetStringTest, for_constructed_from_string_should_return_same_value)
    {
        const Variant variant(std::string("foo"));
        ASSERT_EQ(variant.getString(), "foo");
    }

    TEST(ESMVariantSetTypeTest, for_unknown_should_reset_data)
    {
        Variant variant(int{42});
        variant.setType(ESM::VT_Unknown);
        ASSERT_THROW(variant.getInteger(), std::runtime_error);
    }

    TEST(ESMVariantSetTypeTest, for_none_should_reset_data)
    {
        Variant variant(int{42});
        variant.setType(ESM::VT_None);
        ASSERT_THROW(variant.getInteger(), std::runtime_error);
    }

    TEST(ESMVariantSetTypeTest, for_same_type_should_not_change_value)
    {
        Variant variant(int{42});
        variant.setType(ESM::VT_Long);
        ASSERT_EQ(variant.getInteger(), 42);
    }

    TEST(ESMVariantSetTypeTest, for_float_replaced_by_int_should_cast_float_to_int)
    {
        Variant variant(float{2.7f});
        variant.setType(ESM::VT_Int);
        ASSERT_EQ(variant.getInteger(), 2);
    }

    TEST(ESMVariantSetTypeTest, for_string_replaced_by_int_should_set_default_initialized_data)
    {
        Variant variant(std::string("foo"));
        variant.setType(ESM::VT_Int);
        ASSERT_EQ(variant.getInteger(), 0);
    }

    TEST(ESMVariantSetTypeTest, for_default_constructed_replaced_by_float_should_set_default_initialized_value)
    {
        Variant variant;
        variant.setType(ESM::VT_Float);
        ASSERT_EQ(variant.getInteger(), 0.0f);
    }

    TEST(ESMVariantSetTypeTest, for_float_replaced_by_short_should_cast_data_to_int)
    {
        Variant variant(float{2.7f});
        variant.setType(ESM::VT_Short);
        ASSERT_EQ(variant.getInteger(), 2);
    }

    TEST(ESMVariantSetTypeTest, for_float_replaced_by_long_should_cast_data_to_int)
    {
        Variant variant(float{2.7f});
        variant.setType(ESM::VT_Long);
        ASSERT_EQ(variant.getInteger(), 2);
    }

    TEST(ESMVariantSetTypeTest, for_int_replaced_by_float_should_cast_data_to_float)
    {
        Variant variant(int{42});
        variant.setType(ESM::VT_Float);
        ASSERT_EQ(variant.getFloat(), 42.0f);
    }

    TEST(ESMVariantSetTypeTest, for_int_replaced_by_string_should_set_default_initialized_data)
    {
        Variant variant(int{42});
        variant.setType(ESM::VT_String);
        ASSERT_EQ(variant.getString(), "");
    }

    TEST(ESMVariantSetIntegerTest, for_default_constructed_should_throw_exception)
    {
        Variant variant;
        ASSERT_THROW(variant.setInteger(42), std::runtime_error);
    }

    TEST(ESMVariantSetIntegerTest, for_unknown_should_throw_exception)
    {
        Variant variant;
        variant.setType(ESM::VT_Unknown);
        ASSERT_THROW(variant.setInteger(42), std::runtime_error);
    }

    TEST(ESMVariantSetIntegerTest, for_default_int_should_change_value)
    {
        Variant variant(int{13});
        variant.setInteger(42);
        ASSERT_EQ(variant.getInteger(), 42);
    }

    TEST(ESMVariantSetIntegerTest, for_int_should_change_value)
    {
        Variant variant;
        variant.setType(ESM::VT_Int);
        variant.setInteger(42);
        ASSERT_EQ(variant.getInteger(), 42);
    }

    TEST(ESMVariantSetIntegerTest, for_short_should_change_value)
    {
        Variant variant;
        variant.setType(ESM::VT_Short);
        variant.setInteger(42);
        ASSERT_EQ(variant.getInteger(), 42);
    }

    TEST(ESMVariantSetIntegerTest, for_float_should_change_value)
    {
        Variant variant(float{2.7f});
        variant.setInteger(42);
        ASSERT_EQ(variant.getFloat(), 42.0f);
    }

    TEST(ESMVariantSetIntegerTest, for_string_should_throw_exception)
    {
        Variant variant(std::string{});
        ASSERT_THROW(variant.setInteger(42), std::runtime_error);
    }

    TEST(ESMVariantSetFloatTest, for_default_constructed_should_throw_exception)
    {
        Variant variant;
        ASSERT_THROW(variant.setFloat(2.7f), std::runtime_error);
    }

    TEST(ESMVariantSetFloatTest, for_unknown_should_throw_exception)
    {
        Variant variant;
        variant.setType(ESM::VT_Unknown);
        ASSERT_THROW(variant.setFloat(2.7f), std::runtime_error);
    }

    TEST(ESMVariantSetFloatTest, for_default_int_should_change_value)
    {
        Variant variant(int{13});
        variant.setFloat(2.7f);
        ASSERT_EQ(variant.getInteger(), 2);
    }

    TEST(ESMVariantSetFloatTest, for_int_should_change_value)
    {
        Variant variant;
        variant.setType(ESM::VT_Int);
        variant.setFloat(2.7f);
        ASSERT_EQ(variant.getInteger(), 2);
    }

    TEST(ESMVariantSetFloatTest, for_short_should_change_value)
    {
        Variant variant;
        variant.setType(ESM::VT_Short);
        variant.setFloat(2.7f);
        ASSERT_EQ(variant.getInteger(), 2);
    }

    TEST(ESMVariantSetFloatTest, for_float_should_change_value)
    {
        Variant variant(float{2.7f});
        variant.setFloat(3.14f);
        ASSERT_EQ(variant.getFloat(), 3.14f);
    }

    TEST(ESMVariantSetFloatTest, for_string_should_throw_exception)
    {
        Variant variant(std::string{});
        ASSERT_THROW(variant.setFloat(2.7f), std::runtime_error);
    }

    TEST(ESMVariantSetStringTest, for_default_constructed_should_throw_exception)
    {
        Variant variant;
        ASSERT_THROW(variant.setString("foo"), std::bad_variant_access);
    }

    TEST(ESMVariantSetStringTest, for_unknown_should_throw_exception)
    {
        Variant variant;
        variant.setType(ESM::VT_Unknown);
        ASSERT_THROW(variant.setString("foo"), std::bad_variant_access);
    }

    TEST(ESMVariantSetStringTest, for_default_int_should_throw_exception)
    {
        Variant variant(int{13});
        ASSERT_THROW(variant.setString("foo"), std::bad_variant_access);
    }

    TEST(ESMVariantSetStringTest, for_int_should_throw_exception)
    {
        Variant variant;
        variant.setType(ESM::VT_Int);
        ASSERT_THROW(variant.setString("foo"), std::bad_variant_access);
    }

    TEST(ESMVariantSetStringTest, for_short_should_throw_exception)
    {
        Variant variant;
        variant.setType(ESM::VT_Short);
        ASSERT_THROW(variant.setString("foo"), std::bad_variant_access);
    }

    TEST(ESMVariantSetStringTest, for_float_should_throw_exception)
    {
        Variant variant(float{2.7f});
        ASSERT_THROW(variant.setString("foo"), std::bad_variant_access);
    }

    TEST(ESMVariantSetStringTest, for_string_should_change_value)
    {
        Variant variant(std::string("foo"));
        variant.setString("bar");
        ASSERT_EQ(variant.getString(), "bar");
    }

    struct WriteToESMTestCase
    {
        Variant mVariant;
        Variant::Format mFormat;
        std::size_t mDataSize {};
    };

    std::string write(const Variant& variant, const Variant::Format format)
    {
        std::ostringstream out;
        ESM::ESMWriter writer;
        writer.save(out);
        switch (format)
        {
            case Variant::Format_Gmst:
            {
                writer.startRecord("GMST");
                ESM3::GameSetting gmst;
                gmst.mId = "gmst_test";
                gmst.mValue = variant;
                gmst.save(writer, false);
                writer.endRecord("GMST");
                break;
            }
            case Variant::Format_Global:
            {
                writer.startRecord("GLOB");
                ESM3::Global glob;
                glob.mId = "glob_test";
                glob.mValue = variant;
                glob.save(writer, false);
                writer.endRecord("GLOB");
                break;
            }
            case Variant::Format_Info:
            {
                writer.startRecord("INFO");
                ESM3::DialInfo info;
                info.mId = "info_test";
                ESM3::DialInfo::SelectStruct ss;
                ss.mSelectRule = "me";
                ss.mValue = variant;
                info.mSelects.push_back(ss);
                info.save(writer, false);
                writer.endRecord("INFO");
                break;
            }
            case Variant::Format_Local:
            {
                writer.startRecord("GSCR");
                ESM3::GlobalScript script;
                script.mId = "script_test";
                script.mRunning = 0;
                script.mTargetRef.unset();
                script.mLocals.mVariables.push_back(std::make_pair("test_var", variant));
                script.save(writer);
                writer.endRecord("GSCR");
                break;
            }
            default:
                break;
        }
        writer.close();
        return out.str();
    }

    Variant read(const Variant::Format format, const std::string& data)
    {
        Variant result;
        ESM3::Reader* reader = new ESM3::Reader(std::make_shared<std::istringstream>(data), "");
        reader->getRecordHeader();
        switch (format)
        {
            case Variant::Format_Gmst:
            {
                reader->getSubRecordHeader(); // NAME
                std::string id;
                reader->getZString(id);
                break;
            }
            case Variant::Format_Global:
            {
                reader->getSubRecordHeader(); // NAME
                std::string id;
                reader->getZString(id);
                // FIXME: we need to read the sub-record header before reading the variant
                reader->getSubRecordHeader(); // FNAM
                break;
            }
            case Variant::Format_Info:
            {
                reader->getSubRecordHeader(); // INAM
                std::string id;
                reader->getZString(id);
                reader->getSubRecordHeader(); // PNAM
                reader->skipSubRecordData();
                reader->getSubRecordHeader(); // NNAM
                reader->skipSubRecordData();
                reader->getSubRecordHeader(); // DATA
                reader->skipSubRecordData();
                reader->getSubRecordHeader(); // SCVR
                reader->skipSubRecordData();  // mSelectRule
                break;
            }
            case Variant::Format_Local:
            {
                reader->getSubRecordHeader(); // NAME
                std::string id;
                reader->getString(id); // NOTE: string not null terminated
                reader->getSubRecordHeader(); // LOCA
                reader->skipSubRecordData();  // "test_var"
                break;
            }
            default:
                break;
        }

        result.read(*reader, format);
        return result;
    }

    Variant writeAndRead(const Variant& variant, const Variant::Format format, std::size_t dataSize)
    {
        const std::string data = write(variant, format);
        EXPECT_EQ(data.size(), dataSize);
        return read(format, data);
    }

    struct ESMVariantToESMTest : TestWithParam<WriteToESMTestCase> {};

    TEST_P(ESMVariantToESMTest, deserialized_is_equal_to_serialized)
    {
        const auto param = GetParam();
        const auto result = writeAndRead(param.mVariant, param.mFormat, param.mDataSize);
        ASSERT_EQ(param.mVariant, result);
    }

    INSTANTIATE_TEST_SUITE_P(VariantAndData, ESMVariantToESMTest, Values(
        WriteToESMTestCase {Variant(float{2.7f}), Variant::Format_Gmst, /*336*/370},
        WriteToESMTestCase {Variant(std::string("foo")), Variant::Format_Gmst, /*335*/369},
        WriteToESMTestCase {makeVariant(ESM::VT_Int, 42), Variant::Format_Gmst, /*336*/370},
        WriteToESMTestCase {Variant(int{42}), Variant::Format_Global, /*345*/379},
        WriteToESMTestCase {Variant(float{2.7f}), Variant::Format_Global, /*345*/379},
        WriteToESMTestCase {Variant(float{2.7f}), Variant::Format_Info, /*336*/418},
        WriteToESMTestCase {Variant(float{2.7f}), Variant::Format_Local, /*336*/387},
        WriteToESMTestCase {makeVariant(ESM::VT_Short, 42), Variant::Format_Global, /*345*/379},
        WriteToESMTestCase {makeVariant(ESM::VT_Short, 42), Variant::Format_Local, /*334*/385},
        WriteToESMTestCase {makeVariant(ESM::VT_Int, 42), Variant::Format_Info, /*336*/418},
        WriteToESMTestCase {makeVariant(ESM::VT_Int, 42), Variant::Format_Local, /*336*/387}
    ));

    struct ESMVariantToESMNoneTest : TestWithParam<WriteToESMTestCase> {};

    TEST_P(ESMVariantToESMNoneTest, deserialized_is_none)
    {
        const auto param = GetParam();
        const auto result = writeAndRead(param.mVariant, param.mFormat, param.mDataSize);
        ASSERT_EQ(Variant(), result);
    }

    INSTANTIATE_TEST_SUITE_P(VariantAndData, ESMVariantToESMNoneTest, Values(
        WriteToESMTestCase {Variant(), Variant::Format_Gmst, 358}
    ));

    struct ESMVariantWriteToESMFailTest : TestWithParam<WriteToESMTestCase> {};

    TEST_P(ESMVariantWriteToESMFailTest, write_is_not_supported)
    {
        const auto param = GetParam();
        std::ostringstream out;
        ESM::ESMWriter writer;
        writer.save(out);
        ASSERT_THROW(param.mVariant.write(writer, param.mFormat), std::runtime_error);
    }

    INSTANTIATE_TEST_SUITE_P(VariantAndFormat, ESMVariantWriteToESMFailTest, Values(
        WriteToESMTestCase {Variant(), Variant::Format_Global},
        WriteToESMTestCase {Variant(), Variant::Format_Info},
        WriteToESMTestCase {Variant(), Variant::Format_Local},
        WriteToESMTestCase {Variant(int{42}), Variant::Format_Gmst},
        WriteToESMTestCase {Variant(int{42}), Variant::Format_Info},
        WriteToESMTestCase {Variant(int{42}), Variant::Format_Local},
        WriteToESMTestCase {Variant(std::string("foo")), Variant::Format_Global},
        WriteToESMTestCase {Variant(std::string("foo")), Variant::Format_Info},
        WriteToESMTestCase {Variant(std::string("foo")), Variant::Format_Local},
        WriteToESMTestCase {makeVariant(ESM::VT_Unknown), Variant::Format_Global},
        WriteToESMTestCase {makeVariant(ESM::VT_Int, 42), Variant::Format_Global},
        WriteToESMTestCase {makeVariant(ESM::VT_Short, 42), Variant::Format_Gmst},
        WriteToESMTestCase {makeVariant(ESM::VT_Short, 42), Variant::Format_Info}
    ));
}
