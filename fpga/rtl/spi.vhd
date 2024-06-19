library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity SPI is
   port (
      i_reset_n      : in std_logic;
      i_clk          : in std_logic;         
      i_spi_clk      : in std_logic;
      i_spi_sdi      : in std_logic;
      --
      o_datain       : out std_logic_vector (7 downto 0);
      o_datain_clk   : out std_logic;
      o_test         : out std_logic
   );
end entity SPI;

architecture SPI_arch of SPI is
signal s_spi_clk_sync : std_logic_vector(2 downto 0);
signal s_spi_clk      : std_logic;
signal s_last_spi_clk : std_logic;
signal s_idle_count   : integer;
signal s_idle         : std_logic;
signal s_toggle       : std_logic;
signal s_bits         : integer;
signal s_shift_data   : std_logic_vector (7 downto 0);
begin
   spi_sync: process(i_clk, i_reset_n)
   begin
      if i_reset_n = '0'then
      
      elsif rising_edge(i_clk) then
         s_spi_clk_sync(0) <= i_spi_clk;
         s_spi_clk_sync(1) <=s_spi_clk_sync(0);
         s_spi_clk_sync(2) <=s_spi_clk_sync(1);
         s_spi_clk <= s_spi_clk_sync(0) and s_spi_clk_sync(1) and s_spi_clk_sync(2);
      end if;
      
   end process;
   
   spi_proc: process (i_clk, i_reset_n)
   begin
      if i_reset_n = '0' then
         s_last_spi_clk <= '0';
         s_idle_count <= 0;
         s_bits <= 0;
         s_shift_data <= (others => '0');
         s_toggle <= '0';

      elsif rising_edge(i_clk) then
         if s_last_spi_clk /= s_spi_clk then
            s_last_spi_clk <= s_spi_clk;
            s_toggle <= not s_toggle;
            if i_spi_clk = '1' then 
               s_idle_count <= 0;
               s_shift_data <= s_shift_data(6 downto 0) & i_spi_sdi;
               s_bits <= s_bits + 1;
               case s_bits is
               when 7 => 
                  o_datain <= s_shift_data(6 downto 0) & i_spi_sdi;
                  o_datain_clk <= '1';
                  s_bits <= 0;
               when 3 =>
                  o_datain_clk <= '0';
               when others =>
               end case;
            end if;
         elsif i_spi_clk = '0' then 
            if s_idle_count = 6000000 then 
               s_bits <= 0;
            else
               s_idle_count <= s_idle_count + 1;
            end if;
         end if;
      end if;
      o_test <= s_spi_clk;
            
   end process;
   
end architecture SPI_arch;
