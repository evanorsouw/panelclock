library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity fifo is
   generic (
      DEPTH : natural := 3; -- fifo can hold 2^3 entries
      WIDTH : natural := 8  -- number of bits per entry
   );
   port (    
      i_reset_n : in std_logic;
      i_clk     : in std_logic;
      i_wen     : in std_logic;
      i_ren     : in std_logic;      
      i_data    : in std_logic_vector(WIDTH-1 downto 0);
      o_data    : out std_logic_vector(WIDTH-1 downto 0);
      o_full    : out std_logic;
      o_empty   : out std_logic;
      o_count   : out natural
);
end entity;

architecture fifo_arch of fifo is

  signal s_windex    : unsigned(DEPTH-1 downto 0);
  signal s_rindex    : unsigned(DEPTH-1 downto 0);
  signal s_count     : unsigned(DEPTH-1 downto 0);
  
  type tMem is array (0 to 2**DEPTH-1) of std_logic_vector(WIDTH-1 downto 0);
  signal s_mem : tMem; 
  
begin
   process (i_reset_n, i_clk)
   begin
      if i_reset_n = '0' then
         s_windex <= to_unsigned(0, DEPTH);
         s_rindex <= to_unsigned(0, DEPTH);
         s_count <= to_unsigned(0, DEPTH);
         o_full <= '0';
         o_empty <= '0';
         
      elsif rising_edge(i_clk) then

         if i_wen = '1' and i_ren = '0' then
            if s_count < 2**DEPTH -1 then
               s_count <= s_count + 1;
               o_count <= to_integer(s_count + 1);
               o_empty <= '0';
               if s_count = 2**DEPTH-2 then
                  o_full <= '1';
               end if;
            end if;
         end if;
         
         if i_wen = '0' and i_ren = '1' then
            if s_count  > 0 then
               s_count <= s_count - 1;
               o_count <= to_integer(s_count - 1);
               o_full <= '0';
               if s_count = 1 then
                  o_empty <= '1';
               end if;
            end if;
         end if;
                 
         if i_wen = '1' and s_count < (2**DEPTH-1) then
            s_mem(to_integer(s_windex)) <= i_data;
            s_windex <= s_windex + 1;  -- auto wrap due to size being power of 2
         end if;
         if i_ren = '1' and s_count > 0 then
            o_data   <= s_mem(to_integer(s_rindex));
            s_rindex <= s_rindex + 1;  -- auto wrap due to size being power of 2
         end if;
                    
      end if;
         
   end process;
   
   
end architecture fifo_arch;
