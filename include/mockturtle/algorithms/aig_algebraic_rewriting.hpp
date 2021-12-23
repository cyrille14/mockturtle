/*!
  \file aig_algebraic_rewriting.hpp
  \brief AIG algebraric rewriting

  EPFL CS-472 2021 Final Project Option 1
*/

#pragma once

#include "../networks/aig.hpp"
#include "../views/depth_view.hpp"
#include "../views/topo_view.hpp"

//#define INTERN_TEST
//#define PRINT_NTW

namespace mockturtle
{

namespace detail
{

template<class Ntk>
class aig_algebraic_rewriting_impl
{
  using node = typename Ntk::node;
  using signal = typename Ntk::signal;

public:
  aig_algebraic_rewriting_impl( Ntk& ntk )
      : ntk( ntk )
  {
    static_assert( has_level_v<Ntk>, "Ntk does not implement depth interface." );
  }

  std::vector<node> foreach_fanout_node( node const& n )
  {
    std::vector<node> nodOut;
    ntk.foreach_node( [&]( node nd )
                      {
                        ntk.foreach_fanin( nd, [&]( signal sig )
                                           {
                                             if ( ntk.get_node( sig ) == n )
                                               nodOut.push_back( nd );
                                           } );
                      } );

    return nodOut;
  }

  std::string getType( node const& n )
  {
    if ( ntk.is_and( n ) )
      return "AND ";
    else if ( ntk.is_pi( n ) )
      return "PI ";
    else
      return "? ";
  }

  std::string getInv( signal const& sig )
  {
    if ( ntk.is_complemented( sig ) )
      return "! ";
    return "";
  }

  std::vector<signal> foreach_fanout_sig( node const& n )
  {
    std::vector<signal> sigOut;
    ntk.foreach_node( [&]( node nd )
                      {
                        ntk.foreach_fanin( nd, [&]( signal sig )
                                           {
                                             if ( ntk.get_node( sig ) == n )
                                               sigOut.push_back( sig );
                                           } );
                      } );
    ntk.foreach_po( [&]( signal sig )
                    {
                      if ( ntk.get_node( sig ) == n )
                        sigOut.push_back( sig );
                    } );

    return sigOut;
  }

  std::vector<signal> foreach_fanout_sig_node( node const& n )
  {
    std::vector<signal> sigOut;
    ntk.foreach_node( [&]( node nd )
                      {
                        ntk.foreach_fanin( nd, [&]( signal sig )
                                           {
                                             if ( ntk.get_node( sig ) == n )
                                               sigOut.push_back( sig );
                                           } );
                      } );

    return sigOut;
  }

  std::vector<signal> foreach_fanout_PO( node const& n )
  {
    std::vector<signal> sigOut;
    ntk.foreach_po( [&]( signal sig )
                    {
                      if ( ntk.get_node( sig ) == n )
                        sigOut.push_back( sig );
                    } );

    return sigOut;
  }

  void inv_output( node const& n )
  {
    std::vector<signal> outS = foreach_fanout_sig_node( n );
    std::vector<node> nd = foreach_fanout_node( n );

    for ( int i = 0; i < outS.size(); i++ )
    {

      if ( ntk.is_complemented( outS[i] ) )
        ntk.replace_in_node( nd[i], n, outS[i] );
      else
        ntk.replace_in_node( nd[i], n, !outS[i] );
    }
    outS = foreach_fanout_PO( n );

    for ( int i = 0; i < outS.size(); i++ )
    {

      if ( ntk.is_complemented( outS[i] ) )
        ntk.replace_in_outputs( n, outS[i] );
      else
        ntk.replace_in_outputs( n, !outS[i] );
    }
  }

  void test( void )
  {

    ntk.foreach_gate( [&]( node n )
                      {
                        if ( ntk.node_to_index( n ) == 11 )
                        {

                          if ( try_distributivity3lvBis( n ) )
                          {
                            std::cout << "sucess!!!" << std::endl;
                          }
                          else
                          {
                            std::cout << "fail!!!" << std::endl;
                          }

                          //inv_output( n );

                          /*
                          if ( try_distributivity3lv(n) )
                          {
                            std::cout << "sucess!!!" << std::endl;
                          }
                          else
                          {
                            std::cout << "fail!!!" << std::endl;
                          }
                          //*/

                          /*
                          if ( try_absorption( n ) )
                          {
                            std::cout << "sucess!!!" << std::endl;
                          }
                          else
                          {
                            std::cout << "fail!!!" << std::endl;
                          }
                          //*/
                          /*
                          if ( try_distributivity( n ) )
                          {
                            std::cout << "sucess!!!" << std::endl;
                          }
                          else
                          {
                            std::cout << "fail!!!" << std::endl;
                          }
                          //*/
                        }
                      } );
  }

  void print_tework( void )
  {
    ntk.foreach_gate( [&]( node n )
                      {
                        std::vector<signal> sigLv;

                        ntk.foreach_fanin( n, [&]( signal sig )
                                           {
                                             std::string str = "IN: " + getInv( sig );
                                             std::cout << str << ntk.get_node( sig ) << std::endl;
                                           } );

                        std::string str = getType( n );
                        std::cout << "\t" << n << " " << str << std::endl;

                        std::vector<node> tmp = foreach_fanout_node( n );

                        for ( int j = 0; j < tmp.size(); j++ )
                        {
                          std::vector<signal> tmp2;
                          std::vector<node> tmp3;
                          ntk.foreach_fanin( tmp[j], [&]( signal sig )
                                             {
                                               tmp2.push_back( sig );
                                               tmp3.push_back( ntk.get_node( sig ) );
                                             } ); //Extracts the fanin signals

                          ntk.foreach_fanin( tmp[j], [&]( signal sig )
                                             {
                                               if ( ntk.get_node( sig ) == n )
                                               {
                                                 std::string str = "\tOUT: " + getInv( sig );
                                                 std::cout << str << tmp[j] << std::endl;
                                               }
                                             } );
                        }

                        std::vector<signal> sg = foreach_fanout_PO( n );
                        for ( int i = 0; i < sg.size(); i++ )
                          std::cout << "\tOUT: " << getInv( sg[i] ) << "PO" << std::endl;
                      } );
  }

  void run()
  {
///*
#ifdef INTERN_TEST
    print_tework();
    test();
    print_tework();
#else
#ifdef PRINT_NTW
    print_tework();
#endif
    //*/
    bool cont{ true };
    while ( cont )
    {
      cont = false;
      ntk.foreach_gate( [&]( node n )
                        {
                          if ( try_algebraic_rules( n ) )
                          {
                            ntk.update_levels();
                            cont = true;
                          }
                        } );
    }
#ifdef PRINT_NTW
    print_tework();
#endif
#endif
  }

private:
  bool try_algebraic_rules( node n )
  {
    if ( try_associativity( n ) )
      return true;
    if ( try_distributivity( n ) )
      return true;
    if ( try_distributivity3lvBis( n ) )
      return true;

    return false;
  }

  bool try_associativity( node n )
  {

     if ( ntk.is_on_critical_path( n ) == 0 )
      return false;

    std::vector<signal> sigLv1n;

    ntk.foreach_fanin( n, [&]( signal sig )
                       { sigLv1n.push_back( sig ); } );

    if ( sigLv1n.size() != 2 )
      return false;

    if ( ntk.is_pi( ntk.get_node( sigLv1n.at( 0 ) ) ) && ntk.is_pi( ntk.get_node( sigLv1n.at( 1 ) ) ) )
      return false;

    if ( ntk.level( ntk.get_node( sigLv1n.at( 0 ) ) ) > ( ntk.level( ntk.get_node( sigLv1n.at( 1 ) ) ) + 1 ) && ntk.is_complemented( sigLv1n.at( 0 ) ) == 0 )
      std::swap( sigLv1n.at( 0 ), sigLv1n.at( 1 ) );
    else if ( ntk.level( ntk.get_node( sigLv1n.at( 1 ) ) ) > ( ntk.level( ntk.get_node( sigLv1n.at( 0 ) ) ) + 1 ) && ntk.is_complemented( sigLv1n.at( 1 ) ) == 0 )
    { }
    else
      return false;

    std::vector<signal> sigLv2A;

    ntk.foreach_fanin( ntk.get_node( sigLv1n.at( 1 ) ), [&]( signal sig )
                       { sigLv2A.push_back( sig ); } );

    if ( sigLv2A.size() != 2 )
      return false;

    if ( ntk.is_on_critical_path( ntk.get_node( sigLv2A.at( 0 ) ) ) && ( !ntk.is_on_critical_path( ntk.get_node( sigLv2A.at( 1 ) ) ) ) )
      std::swap( sigLv2A.at( 0 ), sigLv2A.at( 1 ) );
    else if ( ntk.is_on_critical_path( ntk.get_node( sigLv2A.at( 1 ) ) ) && ( !ntk.is_on_critical_path( ntk.get_node( sigLv2A.at( 0 ) ) ) ) )
    { }
    else
      return false;

    signal newGateA = ntk.create_and( sigLv1n.at( 0 ), sigLv2A.at( 0 ) );

    signal newGateN = ntk.create_and( newGateA, sigLv2A.at( 1 ) );
    ntk.substitute_node( n, newGateN );

    return true;
  }

  bool try_distributivity3lvBis( node n )
  {
    std::vector<signal> sigLv1n;
    signal z, d;

    ntk.foreach_fanin( n, [&]( signal sig )
                       { sigLv1n.push_back( sig ); } );

    if ( sigLv1n.size() != 2 )
      return false;

    if ( ntk.level( ntk.get_node( sigLv1n[0] ) ) > ntk.level( ntk.get_node( sigLv1n[1] ) ) )
    {
      z = sigLv1n[0];
      d = sigLv1n[1];
    }
    else
    {
      z = sigLv1n[1];
      d = sigLv1n[0];
    }

    if ( ( !ntk.is_complemented( z ) ) || ( ( ntk.level( ntk.get_node( z ) ) - ntk.level( ntk.get_node( d ) ) ) <= 3 ) )
      return false;

    node Z = ntk.get_node( z );
    std::vector<signal> sigLv2Z;
    signal w, c;

    ntk.foreach_fanin( Z, [&]( signal sig )
                       { sigLv2Z.push_back( sig ); } );

    if ( sigLv2Z.size() != 2 )
      return false;

    if ( ntk.level( ntk.get_node( sigLv2Z[0] ) ) > ntk.level( ntk.get_node( sigLv2Z[1] ) ) )
    {
      w = sigLv2Z[0];
      c = sigLv2Z[1];
    }
    else
    {
      w = sigLv2Z[1];
      c = sigLv2Z[0];
    }

    if ( ( !ntk.is_complemented( w ) ) || ( ( ntk.level( ntk.get_node( w ) ) - ntk.level( ntk.get_node( c ) ) ) <= 0 ) )
      return false;

    node W = ntk.get_node( w );
    std::vector<signal> sigLv3W;
    signal a, b;

    ntk.foreach_fanin( W, [&]( signal sig )
                       { sigLv3W.push_back( sig ); } );

    if ( sigLv3W.size() != 2 )
      return false;

    if ( ntk.level( ntk.get_node( sigLv3W[0] ) ) > ntk.level( ntk.get_node( sigLv3W[1] ) ) )
    {
      b = sigLv3W[0];
      a = sigLv3W[1];
    }
    else
    {
      b = sigLv3W[1];
      a = sigLv3W[0];
    }

    if ( ( ntk.level( ntk.get_node( b ) ) - ntk.level( ntk.get_node( a ) ) <= 0 ) )
      return false;
    signal nw = ntk.create_and( a, d );
    signal nz = ntk.create_and( nw, b );
    signal nf = ntk.create_and( !c, d );
    signal nn = ntk.create_and( !nz, !nf );

    ntk.substitute_node( n, !nn );

    return true;
  }

  bool try_distributivity( node n )
  {
    std::vector<signal> sigLv1n;

    ntk.foreach_fanin( n, [&]( signal sig )
                       { sigLv1n.push_back( sig ); } );

    if ( sigLv1n.size() != 2 )
      return false;

    if ( ntk.fanout_size( ntk.get_node( sigLv1n.at( 0 ) ) ) != 1 || ntk.fanout_size( ntk.get_node( sigLv1n.at( 1 ) ) ) != 1 )
      return false;

    std::vector<signal> sigLv2A, sigLv2B;

    ntk.foreach_fanin( ntk.get_node( sigLv1n.at( 0 ) ), [&]( signal sig )
                       { sigLv2A.push_back( sig ); } );
    ntk.foreach_fanin( ntk.get_node( sigLv1n.at( 1 ) ), [&]( signal sig )
                       { sigLv2B.push_back( sig ); } );

    if ( sigLv2A.size() != 2 || sigLv2B.size() != 2 )
      return false;

    if ( ntk.node_to_index( ntk.get_node( sigLv2A.at( 0 ) ) ) == ntk.node_to_index( ntk.get_node( sigLv2B.at( 0 ) ) ) )
    { }
    else if ( ntk.node_to_index( ntk.get_node( sigLv2A.at( 0 ) ) ) == ntk.node_to_index( ntk.get_node( sigLv2B.at( 1 ) ) ) )
      std::swap( sigLv2B.at( 0 ), sigLv2B.at( 1 ) );
    else if ( ntk.node_to_index( ntk.get_node( sigLv2A.at( 1 ) ) ) == ntk.node_to_index( ntk.get_node( sigLv2B.at( 0 ) ) ) )
      std::swap( sigLv2A.at( 0 ), sigLv2A.at( 1 ) );
    else if ( ntk.node_to_index( ntk.get_node( sigLv2A.at( 1 ) ) ) == ntk.node_to_index( ntk.get_node( sigLv2B.at( 1 ) ) ) )
    {
      std::swap( sigLv2A.at( 0 ), sigLv2A.at( 1 ) );
      std::swap( sigLv2B.at( 0 ), sigLv2B.at( 1 ) );
    }
    else
      return false;

    if ( ntk.is_complemented( sigLv2A.at( 0 ) ) != ntk.is_complemented( sigLv2B.at( 0 ) ) )
      return false;

    if ( ntk.is_complemented( sigLv1n.at( 0 ) ) && ntk.is_complemented( sigLv1n.at( 1 ) ) )
    { 
      sigLv2A.at( 1 ) = !sigLv2A.at( 1 );
      sigLv2B.at( 1 ) = !sigLv2B.at( 1 );

      signal newGateB = ntk.create_nand( sigLv2A.at( 1 ), sigLv2B.at( 1 ) );
      signal newGateN = ntk.create_and( sigLv2A.at( 0 ), newGateB );
      ntk.substitute_node( n, !newGateN );
    }
    else
    { 
      if ( ntk.is_complemented( sigLv1n.at( 0 ) ) )
        sigLv2A.at( 1 ) = !sigLv2A.at( 1 );
      if ( ntk.is_complemented( sigLv1n.at( 1 ) ) )
        sigLv2B.at( 1 ) = !sigLv2B.at( 1 );

      signal newGateB = ntk.create_and( sigLv2A.at( 1 ), sigLv2B.at( 1 ) );
      signal newGateN = ntk.create_and( sigLv2A.at( 0 ), newGateB );
      ntk.substitute_node( n, newGateN );
    }

    return true;
  }

private:
  Ntk& ntk;
};

} // namespace detail

template<class Ntk>
void aig_algebraic_rewriting( Ntk& ntk )
{
  static_assert( std::is_same_v<typename Ntk::base_type, aig_network>, "Ntk is not an AIG" );

  depth_view dntk{ ntk };
  detail::aig_algebraic_rewriting_impl p( dntk );
  p.run();
}

} // namespace mockturtle