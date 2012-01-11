#ifndef OPENVPN_ADDR_IP_H
#define OPENVPN_ADDR_IP_H

#include <string>

#include <boost/asio.hpp>

#include <openvpn/common/exception.hpp>
#include <openvpn/common/ostream.hpp>
#include <openvpn/addr/ipv4.hpp>
#include <openvpn/addr/ipv6.hpp>

namespace openvpn {
  namespace IP {

    OPENVPN_SIMPLE_EXCEPTION(ip_addr_unspecified);
    OPENVPN_SIMPLE_EXCEPTION(ip_addr_version_inconsistency);
    OPENVPN_SIMPLE_EXCEPTION(ip_render_exception);
    OPENVPN_EXCEPTION(ip_parse_exception);

    class Addr
    {
    public:
      enum Version { UNSPEC, V4, V6 };

      Addr()
      {
	ver = UNSPEC;
      }

      static std::string validate(const std::string& ipstr, const char *title = NULL)
      {
	Addr a = from_string(ipstr, title);
	return a.to_string();
      }

      static Addr from_string(const std::string& ipstr, const char *title = NULL)
      {
	boost::system::error_code ec;
	boost::asio::ip::address a = boost::asio::ip::address::from_string(ipstr, ec);
	if (ec)
	  {
	    if (!title)
	      title = "";
	    OPENVPN_THROW(ip_parse_exception, "error parsing " << title << " IP address '" << ipstr << "' : " << ec.message());
	  }
	return from_asio(a);
      }

      static Addr from_ipv4(const IPv4::Addr& addr)
      {
	Addr a;
	a.ver = V4;
	a.u.v4 = addr;
	return a;
      }

      static Addr from_ipv6(const IPv6::Addr& addr)
      {
	Addr a;
	a.ver = V6;
	a.u.v6 = addr;
	return a;
      }

      std::string to_string() const
      {
	if (ver != UNSPEC)
	  {
	    const boost::asio::ip::address a = to_asio();
	    boost::system::error_code ec;
	    std::string ret = a.to_string(ec);
	    if (ec)
	      throw ip_render_exception();
	    return ret;
	  }
	else
	  return "UNSPEC";
      }

      static Addr from_asio(const boost::asio::ip::address& addr)
      {
	if (addr.is_v4())
	  {
	    Addr a;
	    a.ver = V4;
	    a.u.v4 = IPv4::Addr::from_asio(addr.to_v4());
	    return a;
	  }
	else if (addr.is_v6())
	  {
	    Addr a;
	    a.ver = V6;
	    a.u.v6 = IPv6::Addr::from_asio(addr.to_v6());
	    return a;
	  }
	else
	  throw ip_addr_unspecified();
      }

      boost::asio::ip::address to_asio() const
      {
	switch (ver)
	  {
	  case V4:
	    return boost::asio::ip::address_v4(u.v4.to_asio());
	  case V6:
	    return boost::asio::ip::address_v6(u.v6.to_asio());
	  default:
	    throw ip_addr_unspecified();
	  }
      }

      Addr operator&(const Addr& other) const {
	if (ver != other.ver)
	  throw ip_addr_version_inconsistency();
	switch (ver)
	  {
	  case V4:
	    {
	      Addr ret;
	      ret.ver = V4;
	      ret.u.v4 = u.v4 & other.u.v4;
	      return ret;
	    }
	  case V6:
	    {
	      Addr ret;
	      ret.ver = V6;
	      ret.u.v6 = u.v6 & other.u.v6;
	      return ret;
	    }
	  default:
	    throw ip_addr_unspecified();
	  }
      }

      Addr operator|(const Addr& other) const {
	if (ver != other.ver)
	  throw ip_addr_version_inconsistency();
	switch (ver)
	  {
	  case V4:
	    {
	      Addr ret;
	      ret.ver = V4;
	      ret.u.v4 = u.v4 | other.u.v4;
	      return ret;
	    }
	  case V6:
	    {
	      Addr ret;
	      ret.ver = V6;
	      ret.u.v6 = u.v6 | other.u.v6;
	      return ret;
	    }
	  default:
	    throw ip_addr_unspecified();
	  }
      }

      void reset_ipv4_from_uint32(const IPv4::Addr::base_type addr)
      {
	ver = V4;
	u.v4 = IPv4::Addr::from_uint32(addr);
      }

      bool unspecified() const
      {
	switch (ver)
	  {
	  case V4:
	    return u.v4.unspecified();
	  case V6:
	    return u.v6.unspecified();
	  default:
	    return true;
	  }
      }

    private:
      union {
	IPv4::Addr v4;
	IPv6::Addr v6;
      } u;

      Version ver;
    };
    OPENVPN_OSTREAM(Addr, to_string)

    struct AddrMaskPair
    {
      Addr addr;
      Addr netmask;

      std::string to_string() const
      {
	return addr.to_string() + "/" + netmask.to_string();
      }
    };
    OPENVPN_OSTREAM(AddrMaskPair, to_string)

  }
} // namespace openvpn

#endif // OPENVPN_ADDR_IP_H
