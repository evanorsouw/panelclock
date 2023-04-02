library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity byte_demux is
	port ( 	
		byte      : in std_logic_vector (7 downto 0);
		addr      : in std_logic_vector (2 downto 0);
		--
		q         : out std_logic
	);
end entity byte_demux;

architecture byte_demux_arch of byte_demux is	
begin
   
   p_mux : process(byte,addr)
   begin
      case addr is
         when "000" => q <= byte(7);				
         when "001" => q <= byte(6);				
         when "010" => q <= byte(5);				
         when "011" => q <= byte(4);				
         when "100" => q <= byte(3);				
         when "101" => q <= byte(2);				
         when "110" => q <= byte(1);				
         when "111" => q <= byte(0);				
      end case;
   end process;
   
end architecture byte_demux_arch;
