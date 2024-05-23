library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity SPI is
   port (
      i_reset_n      : in std_logic;
      i_clkx         : in std_logic;
      i_clk          : in std_logic;
      i_spi_clk      : in std_logic;
      i_spi_sdi      : in std_logic;
      --
      o_datain       : out std_logic_vector (7 downto 0);
      o_datain_clk   : out std_logic
   );
end entity SPI;

architecture SPI_arch of SPI is
begin
   clock_proc: process (i_clkx, i_reset_n)
   begin
      if i_reset_n = '0' then
        o_datain <= "00000000";
        o_datain_clk <= '0';

      elsif rising_edge(i_clkx) then
        

      end if;

   end process;
end architecture SPI_arch;



