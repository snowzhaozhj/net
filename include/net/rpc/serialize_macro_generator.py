import sys

n = int(sys.argv[1])

print("""#ifndef NET_INCLUDE_NET_RPC_SERIALIZE_MACRO_HPP_
#define NET_INCLUDE_NET_RPC_SERIALIZE_MACRO_HPP_""")

for i in range(1, n + 1):
    field_num_list = range(1, i + 1)
    print("""
#define SERIALIZE{0}(cls, {1}) \\
  inline net::Serializer &operator<<(net::Serializer &s, const cls &obj) {{\\
    s << {2};                  \\
    return s;                  \\
  }}                           \\
  inline net::Serializer &operator>>(net::Serializer &s, cls &obj) {{\\
    s >> {3};                    \\
    return s;                    \\
  }}""".format(
        i,
        ", ".join(['f' + str(field_num) for field_num in field_num_list]),
        " << ".join(['obj.f' + str(field_num) for field_num in field_num_list]),
        " >> ".join(['obj.f' + str(field_num) for field_num in field_num_list]),
    ))

print()
print("#endif // NET_INCLUDE_NET_RPC_SERIALIZE_MACRO_HPP_")
