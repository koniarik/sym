#include "expr.hpp"

#include <emlabcpp/algorithm.h>
#include <emlabcpp/experimental/decompose.h>
#include <rapidcheck.h>

namespace rc
{

template < typename T >
struct variant_gen;

template < typename... Ts >
struct variant_gen< std::variant< Ts... > >
{
        static auto gen()
        {
                return gen::oneOf( gen_val< Ts >()... );
        };

        template < typename T >
        static Gen< std::variant< Ts... > > gen_val()
        {
                return gen::map( gen::arbitrary< T >(), [&]( T val ) -> std::variant< Ts... > {
                        return std::move( val );
                } );
        }
};

template < typename T >
struct uvptr_gen;

template < typename... Ts >
struct uvptr_gen< std::variant< std::unique_ptr< Ts >... > >
{
        static auto gen()
        {
                return gen::oneOf( gen_val< Ts >()... );
        };

        template < typename T >
        static Gen< std::variant< std::unique_ptr< Ts >... > > gen_val()
        {
                return gen::map(
                    gen::arbitrary< T >(),
                    [&]( T val ) -> std::variant< std::unique_ptr< Ts >... > {
                            return std::make_unique< T >( std::move( val ) );
                    } );
        }
};

template < typename >
struct tuple_gen;

template < typename... Ts >
struct tuple_gen< std::tuple< Ts... > >
{
        static auto gen()
        {
                return gen::tuple( gen::arbitrary< std::decay_t< Ts > >()... );
        }
};

template < typename T >
struct struct_gen
{
        static Gen< T > arbitrary()
        {
                using tup_type = decltype( emlabcpp::decompose( std::declval< T& >() ) );

                return gen::map( tuple_gen< tup_type >::gen(), [&]( auto tpl ) -> T {
                        return std::apply(
                            [&]( auto... args ) {
                                    return T{ std::move( args )... };
                            },
                            std::move( tpl ) );
                } );
        }
};

template <>
struct Arbitrary< sym::constant >
{
        static Gen< sym::constant > arbitrary()
        {
                return gen::map( gen::inRange< int >( 2, 10 ), [&]( int val ) {
                        return sym::constant{ val };
                } );
        }
};

template <>
struct Arbitrary< sym::bin_op > : struct_gen< sym::bin_op >
{
};

template <>
struct Arbitrary< sym::param >
{

        static Gen< sym::param > arbitrary()
        {
                return gen::map( gen::inRange< std::size_t >( 0, 4 ), [&]( std::size_t val ) {
                        return sym::param{ .i = val };
                } );
        }
};

template <>
struct Arbitrary< sym::bin_op_t >
{
        static Gen< sym::bin_op_t > arbitrary()
        {
                std::vector< sym::bin_op_t > opts = {
                    sym::bin_op_t::ADD,
                    sym::bin_op_t::MINUS,
                    sym::bin_op_t::MULT,
                    sym::bin_op_t::DIV,
                };
                return gen::elementOf( opts );
        }
};

template <>
struct Arbitrary< sym::expr >
{
        static Gen< sym::expr > arbitrary()
        {
                return gen::oneOf(
                    gen::map(
                        gen::arbitrary< sym::constant >(),
                        [&]( auto c ) {
                                return sym::to_expr( c );
                        } ),
                    gen::lazy( []() -> decltype( auto ) {
                            return uvptr_gen< sym::expr >::gen();
                    } ) );
        }
};

}  // namespace rc

static constexpr std::size_t bad_n = 8;

float eval( sym::expr const& e, std::span< float > data );

float eval( sym::constant const& c, std::span< float > )
{
        return c.val;
}

float eval( sym::bin_op const& op, std::span< float > data )
{

        float lh = eval( op.lh, data );
        float rh = eval( op.rh, data );

        switch ( op.op ) {
        case sym::bin_op_t::ADD:
                return lh + rh;
                break;
        case sym::bin_op_t::MINUS:
                return lh - rh;
                break;
        case sym::bin_op_t::MULT:
                return lh * rh;
                break;
        case sym::bin_op_t::DIV:
                if ( rh == 0 )
                        throw std::logic_error{ "div by 0" };
                return lh / rh;
                break;
        }
        return 0.F;
}

float eval( sym::param const& p, std::span< float > data )
{
        return data[p.i];
}

float eval( sym::expr const& e, std::span< float > data )
{
        return std::visit(
            [&]( auto& it ) {
                    return eval( *it, data );
            },
            e );
}

int main()
{
        rc::check( []( sym::expr const& e ) {
                std::array< float, 5 > data{ 1, 2, 3, 4, 5 };
                eval( e, data );
        } );

        return 0;
}
