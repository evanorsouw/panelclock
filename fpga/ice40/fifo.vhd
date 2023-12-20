library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity fifo is
   generic (
      FIFO_DEPTH : integer := 3; -- fifo can hold 2^3 entries
      FIFO_WIDTH : natural := 8  -- number of bits per entry
   );
   port (    
      i_reset   : in std_logic;
      i_clk     : in std_logic;
      i_wen     : in std_logic;
      i_ren     : in std_logic;
      i_data    : in std_logic_vector(FIFO_WIDTH-1 downto 0);
      o_data    : out std_logic_vector(FIFO_WIDTH-1 downto 0);
      o_full    : out std_logic;
      o_empty   : out std_logic
   );
end entity;

architecture fifo_arch of fifo is

  signal wr_index    : integer range 0 to 2**FIFO_DEPTH-1 := 0;
  signal rd_index    : integer range 0 to 2**FIFO_DEPTH-1 := 0;
  signal full        : std_logic;
  signal empty       : std_logic;
  signal fillcount   : integer range 0 to (2**FIFO_DEPTH);
  
  type t_FIFO_DATA is array (0 to 2**FIFO_DEPTH-1) of std_logic_vector(FIFO_WIDTH-1 downto 0);
  signal fifo_data : t_FIFO_DATA := (others => (others => '0'));
  
begin
   process (i_reset, i_clk)
   variable tmp : integer;
   begin
      if i_reset = '0' then
         wr_index   <= 0;
         rd_index   <= 0;
         fillcount  <= 0;        
         
      elsif rising_edge(i_clk) then
         tmp := fillcount;
         if i_wen = '1' and full = '0' then
            tmp := tmp + 1;
            fifo_data(wr_index) <= i_data;
            wr_index <= wr_index + 1;
         end if;
         if i_ren = '1' and empty = '0' then
            tmp := tmp - 1;
            o_data   <= fifo_data(rd_index);
            rd_index <= rd_index + 1;
         end if;
         fillcount <= tmp;
      end if;
   end process;
   
   full  <= '1' when fillcount = (2**FIFO_DEPTH) else '0';
   empty <= '1' when fillcount = 0 else '0';
      
   o_full   <= full;
   o_empty  <= empty;      
   
end architecture fifo_arch;
