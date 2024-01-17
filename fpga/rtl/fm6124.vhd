library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity FM6124 is
   port (    
      i_clkx2       : in std_logic;   -- max 60MHz (2x30MHz)
      i_reset_n   : in std_logic;
      --
      o_addr      : out std_logic_vector (13 downto 0);
      o_read      : out std_logic;
      o_dsp_clk   : out std_logic;
      o_dsp_latch : out std_logic;
      o_dsp_oe_n  : out std_logic;
      o_dsp_addr  : out std_logic_vector (4 downto 0);
      o_dsp_vbl   : out std_logic
   );
end entity FM6124;

architecture FM6124_arch of FM6124 is   
begin
   process (i_clkx2, i_reset_n)
   variable v_clockside      : std_logic;
   variable v_dsp_clk        : std_logic;
   variable v_column_count   : unsigned (6 downto 0);
   variable v_row_count      : unsigned (4 downto 0);
   variable v_row_count_next : unsigned (4 downto 0);
   variable v_pixel_addr     : unsigned (13 downto 0);
   variable v_oe_n           : std_logic;
   variable v_latch          : std_logic;
   variable v_read           : std_logic;
   variable v_bit_count      : unsigned (3 downto 0);
   variable v_waiting        : std_logic;
   variable v_depth_count    : unsigned (15 downto 0);
   variable v_depth_delay    : unsigned (7 downto 0);
   
   begin
      if i_reset_n = '0' then
         v_clockside       := '0';
         v_dsp_clk         := '0';
         v_column_count    := to_unsigned(0, v_column_count'Length);
         v_row_count       := to_unsigned(0, v_row_count'Length);
         v_row_count_next  := to_unsigned(0, v_row_count_next'Length);
         v_pixel_addr      := to_unsigned(0, v_pixel_addr'length);
         v_latch           := '0';
         v_oe_n            := '1';
         v_waiting         := '0';
         v_depth_count     := to_unsigned(1, v_depth_count'Length);
         v_depth_delay     := "10000000";
         o_dsp_vbl         <= '1';
         
      elsif rising_edge(i_clkx2) then
         
         v_clockside := not v_clockside;
         v_dsp_clk := '0';
         v_latch  := '0';
         v_read := '0';                 
         o_dsp_vbl <= '0';
         
         if v_clockside = '1' then
            if (v_waiting = '1') then
               v_depth_count := v_depth_count - 1;
               if (v_depth_count = 1) then
                  v_read := '1';
               elsif (v_depth_count = 0) then
                  v_waiting := '0';
               end if;
            end if;
            
            if (v_waiting = '0') then
               if (v_column_count < 64) then 
                  v_dsp_clk := '1';
                  v_read := '1';
                  v_pixel_addr := v_pixel_addr + 1;
                  v_column_count := v_column_count + 1;                               
               elsif (v_column_count = 64) then
                  v_oe_n := '1';
                  v_row_count := v_row_count_next;
                  v_column_count := v_column_count + 1;                   
               elsif (v_column_count = 65) then
                  v_latch := '1';
                  v_column_count := v_column_count + 1;                   
               elsif (v_column_count = 66) then
                  v_oe_n := '0';
                  v_waiting := '1';
                  v_depth_count := v_depth_delay * 67 - 66;
                  if v_depth_count = 1 then
                     v_read := '1';
                  end if;               
                  v_row_count_next := v_row_count_next + 1;
                  v_column_count := to_unsigned(0, v_column_count'Length);
                  if (v_row_count_next = 0) then
                     v_depth_delay := '0' & v_depth_delay(7 downto 1);
                     if (v_depth_delay = 0) then 
                        -- restart entire refresh cycle
                        v_depth_delay := "10000000";
                        v_pixel_addr := to_unsigned(0, v_pixel_addr'Length);
                        v_row_count_next := to_unsigned(0, v_row_count_next'Length);
                        o_dsp_vbl <= '1';
                     end if;
                  end if;
               end if;
            end if;            
         end if;
         o_addr       <= std_logic_vector(v_pixel_addr);
         o_read       <= v_read;
         o_dsp_clk    <= v_dsp_clk;
         o_dsp_oe_n   <= v_oe_n;
         o_dsp_latch  <= v_latch;
         o_dsp_addr   <= std_logic_vector(v_row_count);
      end if;               
   end process;
   
end architecture FM6124_arch;
            
            
            
            