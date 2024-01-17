library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity brightness_control is
    port (
        valuein:	  in  std_logic_vector(7 downto 0);
        brightness: in std_logic_vector (7 downto 0);
        valueout:	  out std_logic_vector(7 downto 0)
    );
end entity;

architecture brightness_control_arch of brightness_control is
   signal m1 : unsigned (7 downto 0);
   signal m2 : unsigned (8 downto 0);
   signal m3 : unsigned (8 downto 0);
   signal m4 : unsigned (16 downto 0);
begin

   m1 <= unsigned(valuein);
   m2 <= unsigned('0' & brightness);
   m3 <= m2 + 1;
   m4 <= m1 * m3;
   
   valueout <= std_logic_vector(m4(15 downto 8));
end architecture;
