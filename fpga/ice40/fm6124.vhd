library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity FM6124 is
   port (    
      clk60M    : in std_logic;
      reset     : in std_logic;
      --
      addr      : out std_logic_vector (14 downto 0);
      dsp_clk   : out std_logic;
      dsp_latch : out std_logic;
      dsp_oe    : out std_logic;
      dsp_addr  : out std_logic_vector (4 downto 0);
      vbl       : out std_logic
   );
end entity FM6124;

architecture FM6124_arch of FM6124 is   
   signal pixel_clk    : std_logic; 
   
begin
   process (clk60M, reset)
   variable clk30M    : std_logic;
   variable column_count : unsigned (6 downto 0);
   variable row_count    : unsigned (4 downto 0);
   variable pixel_addr   : unsigned (14 downto 0);
   variable oe           : std_logic;
   variable latch        : std_logic;
   variable bit_count    : unsigned (3 downto 0);
   variable waiting      : std_logic;
   variable depth_count  : unsigned (15 downto 0);
   variable depth_delay  : unsigned (7 downto 0);
   
   begin
      if reset = '1' then
         clk30M       := '0';
         column_count := to_unsigned(0, column_count'Length);
         row_count    := to_unsigned(0, row_count'Length);
         pixel_addr   := to_unsigned(0, pixel_addr'length);
         pixel_clk    <= '0';
         latch        := '0';
         oe           := '1';
         waiting      := '0';
         depth_count  := to_unsigned(1, depth_count'Length);
         depth_delay  := "10000000";
         vbl          <= '1';
         
      elsif rising_edge(clk60M) then
         
         clk30M  := not clk30M;
         
         if (clk30M = '0') then
            pixel_clk  <= '0';
                        
         else
            vbl <= '0';
            if (waiting = '1') then
               depth_count := depth_count - 1;
               if (depth_count = 0) then
                  waiting := '0';
                  latch := '0';
               end if;
            else
               if (column_count < 64) then 
                  pixel_clk <= '1';
                  pixel_addr := pixel_addr + 1;
                  column_count := column_count + 1;                   
               elsif (column_count = 64) then
                  latch  := '1';
                  column_count := column_count + 1;                   
               elsif (column_count = 65) then
                  oe := '1';
                  column_count := column_count + 1;                   
               elsif (column_count = 66) then
                  oe := '0';
                  waiting := '1';
                  depth_count := depth_delay * 67 - 66;
                  row_count := row_count + 1;
                  column_count := to_unsigned(0, column_count'Length);
                  if (row_count = 0) then
                     depth_delay := '0' & depth_delay(7 downto 1);
                     if (depth_delay = 0) then 
                        -- restart entire refresh cycle
                        depth_delay := "10000000";
                        pixel_addr := to_unsigned(0, pixel_addr'Length);
                        row_count := to_unsigned(0, row_count'Length);
                        vbl <= '1';
                     end if;
                  end if;
               end if;
            end if;            
         end if;         
      end if;

      addr       <= std_logic_vector(pixel_addr);
      dsp_clk    <= pixel_clk;
      dsp_oe     <= oe;
      dsp_latch  <= latch;
      dsp_addr   <= std_logic_vector(row_count);
      
   end process;
   
   
end architecture FM6124_arch;
            
            
            
            