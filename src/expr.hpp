#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <variant>

namespace sym
{

// nodes

template < typename... Ts >
using uvptr = std::variant< std::unique_ptr< Ts >... >;

struct constant;
struct bin_op;
struct param;

using expr = uvptr< constant, bin_op, param >;

struct constant
{
        float val;
};

struct param
{
        std::size_t i;
};

enum class bin_op_t
{
        ADD,
        MINUS,
        MULT,
        DIV
};

struct bin_op
{
        bin_op_t op;
        expr     lh;
        expr     rh;
};

template < typename T >
expr to_expr( T val )
{
        return expr( std::make_unique< T >( std::move( val ) ) );
}

// output streams

std::ostream& operator<<( std::ostream& os, expr const& e );

std::ostream& operator<<( std::ostream& os, param const& p )
{
        return os << "x[" << p.i << "]";
}

std::ostream& operator<<( std::ostream& os, constant const& c )
{
        return os << c.val;
}

std::ostream& operator<<( std::ostream& os, bin_op_t const& op )
{
        switch ( op ) {
        case bin_op_t::ADD:
                os << "+";
                break;
        case bin_op_t::MINUS:
                os << "-";
                break;
        case bin_op_t::MULT:
                os << "*";
                break;
        case bin_op_t::DIV:
                os << "/";
                break;
        }
        return os;
}

std::ostream& operator<<( std::ostream& os, bin_op const& op )
{
        return os << "(" << op.lh << " " << op.op << " " << op.rh << ")";
}

std::ostream& operator<<( std::ostream& os, expr const& e )
{
        return std::visit(
            [&]( auto& val ) -> std::ostream& {
                    return os << *val;
            },
            e );
}

}  // namespace sym
