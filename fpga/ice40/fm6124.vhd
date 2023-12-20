library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity FM6124 is
   port (    
      i_clk60M    : in std_logic;
      i_reset     : in std_logic;
      --
      o_addr      : out std_logic_vector (14 downto 0);
      o_dsp_clk   : out std_logic;
      o_dsp_latch : out std_logic;
      o_dsp_oe    : out std_logic;
      o_dsp_addr  : out std_logic_vector (4 downto 0);
      o_vbl       : out std_logic
   );
end entity FM6124;

architecture FM6124_arch of FM6124 is   
   signal s_pixel_clk    : std_logic; 
   
begin
   process (i_clk60M, i_reset)
   variable v_clk30M       : std_logic;
   variable v_column_count : unsigned (6 downto 0);
   variable v_row_count    : unsigned (4 downto 0);
   variable v_pixel_addr   : unsigned (14 downto 0);
   variable v_oe           : std_logic;
   variable v_latch        : std_logic;
   variable v_bit_count    : unsigned (3 downto 0);
   variable v_waiting      : std_logic;
   variable v_depth_count  : unsigned (15 downto 0);
   variable v_depth_delay  : unsigned (7 downto 0);
   
   begin
      if i_reset = '0' then
         v_clk30M       := '0';
         v_column_count := to_unsigned(0, v_column_count'Length);
         v_row_count    := to_unsigned(0, v_row_count'Length);
         v_pixel_addr   := to_unsigned(0, v_pixel_addr'length);
         s_pixel_clk    <= '0';
         v_latch        := '0';
         v_oe           := '1';
         v_waiting      := '0';
         v_depth_count  := to_unsigned(1, v_depth_count'Length);
         v_depth_delay  := "10000000";
         o_vbl          <= '1';
         
      elsif rising_edge(i_clk60M) then
         
         v_clk30M  := not v_clk30M;
         
         if (v_clk30M = '0') then
            s_pixel_clk  <= '0';
                        
         else
            o_vbl <= '0';
            if (v_waiting = '1') then
               v_depth_count := v_depth_count - 1;
               if (v_depth_count = 0) then
                  v_waiting := '0';
                  v_latch := '0';
               end if;
            else
               if (v_column_count < 64) then 
                  s_pixel_clk <= '1';
                  v_pixel_addr := v_pixel_addr + 1;
                  v_column_count := v_column_count + 1;                   
               elsif (v_column_count = 64) then
                  v_latch  := '1';
                  v_column_count := v_column_count + 1;                   
               elsif (v_column_count = 65) then
                  v_oe := '1';
                  v_column_count := v_column_count + 1;                   
               elsif (v_column_count = 66) then
                  v_oe := '0';
                  v_waiting := '1';
                  v_depth_count := v_depth_delay * 67 - 66;
                  v_row_count := v_row_count + 1;
                  v_column_count := to_unsigned(0, v_column_count'Length);
                  if (v_row_count = 0) then
                     v_depth_delay := '0' & v_depth_delay(7 downto 1);
                     if (v_depth_delay = 0) then 
                        -- restart entire refresh cycle
                        v_depth_delay := "10000000";
                        v_pixel_addr := to_unsigned(0, v_pixel_addr'Length);
                        v_row_count := to_unsigned(0, v_row_count'Length);
                        o_vbl <= '1';
                     end if;
                  end if;
               end if;
            end if;            
         end if;         
      end if;

      o_addr       <= std_logic_vector(v_pixel_addr);
      o_dsp_clk    <= s_pixel_clk;
      o_dsp_oe     <= v_oe;
      o_dsp_latch  <= v_latch;
      o_dsp_addr   <= std_logic_vector(v_row_count);
      
   end process;
   
   
end architecture FM6124_arch;
            
            
            
            