library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity external_sram is
   generic (
      ADDR_WIDTH  : integer := 14;
      DATA_WIDTH  : integer := 16;
   );
   port (    
      -- data that connects to the external SRAM chip
      wr_ext   : out std_logic;
      oe_ext   : out std_logic
      addr_ext : out std_logic_vector (ADDR_WIDTH-1 downto 0);
      data_out : out std_logic_vector (DATA_WIDTH-1 downto 0);
      -- data to connect to other processes
      clk      : in std_logic      
      addr     : in std_logic_vector (ADDR_WIDTH-1 downto 0);
      data_rd  : out std_logic_vector (DATA_WIDTH-1 downto 0);
      data_wr  : in std_logic_vector (DATA_WIDTH-1 downto 0);
      wr       : in std_logic   -- rising transition triggers write
   );
end entity external_sram;

architecture external_sram_arch of external_sram is   
begin
   variable wr_pending    : std_logic;
   
   process(clk)
   begin
      addr_ext <= addr;
      data_rd <= data;
      if wr = '1' then
         if wr_pending = '0' then
            oe_ext <= '1';
            wr_pending = '1';
         else
      
      else
         data <= data_wr;
         oe_ext <= '0';
         wr_ext <= '1';
         data <= (others => 'Z');        
      end if
      
   end process;
end architecture external_sram_arch;
