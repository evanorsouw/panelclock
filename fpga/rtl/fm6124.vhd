library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity FM6124 is
   port (
      i_clkx2     : in std_logic;   -- max 60MHz (2x30MHz)
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
   signal s_clockside      : std_logic;
   signal s_column_count   : unsigned (6 downto 0);
   signal s_row_count      : unsigned (4 downto 0);
   signal s_depth_count    : unsigned (15 downto 0);
   signal s_depth_delay    : unsigned (7 downto 0);
   signal s_pixel_addr     : unsigned (13 downto 0);

begin
   process (i_clkx2, i_reset_n)
   variable v_dsp_clk      : std_logic;
   variable v_latch        : std_logic;
   variable v_read         : std_logic;
   variable v_waiting      : std_logic;
   variable v_dsp_vbl      : std_logic;  
   variable v_oe_n         : std_logic;
   variable v_pixel_addr   : unsigned(13 downto 0);
   variable v_row_count    : unsigned(4 downto 0);

   begin
      if i_reset_n = '0' then
         s_clockside       <= '0';
         s_column_count    <= to_unsigned(0, s_column_count'Length);
         s_row_count       <= to_unsigned(0, s_row_count'Length);
         s_depth_count     <= to_unsigned(1, s_depth_count'Length);
         s_depth_delay     <= "10000000";      
         s_pixel_addr      <= to_unsigned(0, s_pixel_addr'length);

      elsif rising_edge(i_clkx2) then

         s_clockside <= not s_clockside;
         v_dsp_clk := '0';
         v_latch   := '0';
         v_dsp_vbl := '0';
         v_read    := '0';
         v_waiting := '0';
         v_oe_n    := '0';
         v_pixel_addr := s_pixel_addr;
         v_row_count  := s_row_count;
                  
         if s_depth_count = 0 then
            v_read := '1';
         else
            v_waiting := '1';          
            s_depth_count <= s_depth_count - 1;            
         end if;        
         
         if s_clockside = '0' and v_waiting = '0' then
            if (s_column_count < 64) then
               v_dsp_clk := '1';
               v_pixel_addr := v_pixel_addr + 1;                
               s_column_count <= s_column_count + 1;
            elsif (s_column_count = 64) then
               v_oe_n := '1';
               s_column_count <= s_column_count + 1;
            elsif (s_column_count = 65) then
               v_oe_n := '1';
               v_latch := '1';
               s_column_count <= s_column_count + 1;
            elsif (s_column_count = 66) then
               v_row_count := v_row_count + 1;
               s_depth_count <= s_depth_delay * 67 - 66;
               s_column_count <= to_unsigned(0, s_column_count'Length);
               if (v_row_count = 31) then
                  if (s_depth_delay = "00000001") then
                     -- restart entire refresh cycle
                     s_depth_delay <= "10000000";
                     v_pixel_addr := to_unsigned(0, v_pixel_addr'Length);
                     v_dsp_vbl := '1';
                  else
                     s_depth_delay <= '0' & s_depth_delay(7 downto 1);
                  end if;
               end if;
            end if;
            s_pixel_addr <= v_pixel_addr;
            s_row_count  <= v_row_count;
            o_addr       <= std_logic_vector(v_pixel_addr);
            o_dsp_oe_n   <= v_oe_n;
            o_dsp_addr   <= std_logic_vector(v_row_count);
            o_dsp_vbl    <= v_dsp_vbl;
         end if;
         
         o_dsp_latch  <= v_latch;
         o_dsp_clk    <= v_dsp_clk;
         o_read       <= v_read;

      end if;
   end process;

end architecture FM6124_arch;



