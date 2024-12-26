/* -*- c++ -*- */
/* 
 * Copyright 2020 gr-InterfaceJRO_7 author.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_INTERFACEJRO_7_INTERFACEJRO_7_IMPL_H
#define INCLUDED_INTERFACEJRO_7_INTERFACEJRO_7_IMPL_H

#include <InterfaceJRO_7/InterfaceJRO_7.h>
#include <InterfaceJRO_7/ArgProcessor.h>
#include <boost/asio.hpp>

namespace gr {
  namespace InterfaceJRO_7 {

    class InterfaceJRO_7_impl : public InterfaceJRO_7
    {
     private:
      const char* c_path_file;
      const std::string c_mac_block;
      int c_port_block;
      const std::string c_ip_host;

      boost::asio::ip::udp::socket* local_socket;	

     public:
      InterfaceJRO_7_impl(const char* path_file, const std::string mac_block, int port_block, const std::string ip_host);
      ~InterfaceJRO_7_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace InterfaceJRO_7
} // namespace gr

#endif /* INCLUDED_INTERFACEJRO_7_INTERFACEJRO_7_IMPL_H */

